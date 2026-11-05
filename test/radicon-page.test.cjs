'use strict';
const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const html = fs.readFileSync(path.join(__dirname, '../data/index-radicon.html'), 'utf8');
const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];

function controller(preview = false)
{
    const elements = new Map();
    function element(id)
    {
        if (elements.has(id)) return elements.get(id);
        const events = new Map();
        const capture = new Set();
        const el =
        {
            value: id === 'speed' ? '30' : '0',
            textContent: '',
            classList: {toggle() {}},
            setAttribute() {}, removeAttribute() {},
            addEventListener(name, handler)
            {
                if (!events.has(name)) events.set(name, []);
                events.get(name).push(handler);
            },
            emit(name, args = {})
            {
                for (const handler of events.get(name) || [])
                {
                    handler({button: 0, pointerId: 1, preventDefault() {}, ...args});
                }
            },
            setPointerCapture(id) {capture.add(id);},
            hasPointerCapture(id) {return capture.has(id);},
            releasePointerCapture(id) {capture.delete(id); el.emit('lostpointercapture', {pointerId: id});}
        };
        elements.set(id, el);
        return el;
    }
    const documentEvents = element('document');
    const windowEvents = element('window');
    const calls = [];
    const intervals = [];
    const timers = new Map();
    let timerId = 0;
    let fetchHandler = async url => ({ok: true, text: async () => url});
    const document = {hidden: false, getElementById: element, addEventListener: documentEvents.addEventListener};
    const context = vm.createContext(
    {
        document,
        window: {addEventListener: windowEvents.addEventListener},
        location: {protocol: preview ? 'file:' : 'http:', hostname: preview ? '' : '192.168.100.1'},
        AbortController,
        fetch: async url => {calls.push(url); return fetchHandler(url);},
        setTimeout(fn, ms) {const id = ++timerId; timers.set(id, {fn, ms}); return id;},
        clearTimeout(id) {timers.delete(id);},
        setInterval(fn) {intervals.push(fn);}
    });
    vm.runInContext(script, context);
    async function flush()
    {
        for (let i = 0; i < 60; ++i)
        {
            await Promise.resolve();
            for (const [id, timer] of timers)
            {
                if (timer.ms === 0) {timers.delete(id); timer.fn();}
            }
        }
    }
    return {element, calls, flush, document,
        state: name => vm.runInContext(name, context),
        fetchWith: fn => {fetchHandler = fn;},
        heartbeat: () => intervals.forEach(fn => fn()),
        blur: () => windowEvents.emit('blur'),
        visibility: () => documentEvents.emit('visibilitychange')};
}

test('file preview never sends commands', async () =>
{
    const app = controller(true);
    app.element('forward').emit('pointerdown');
    app.heartbeat();
    await app.flush();
    assert.equal(app.calls.length, 0);
    assert.equal(app.element('mode').textContent, 'プレビュー');
});

test('starts stopped; backward sends negative power; release stops power first', async () =>
{
    const app = controller();
    await app.flush();
    assert.deepEqual(app.calls, ['/power?power=0', '/steering?steering=0.00']);
    app.calls.length = 0;
    app.element('backward').emit('pointerdown');
    await app.flush();
    assert.ok(app.calls.includes('/power?power=-0.30'));
    app.calls.length = 0;
    app.element('backward').emit('pointerup');
    await app.flush();
    assert.deepEqual(app.calls, ['/power?power=0', '/steering?steering=0.00']);
    assert.equal(app.state('direction'), 0);
});

test('left steering and forward power use existing endpoints', async () =>
{
    const app = controller();
    await app.flush();
    app.element('steering').value = '-80';
    app.element('steering').emit('input');
    await app.flush();
    app.element('forward').emit('pointerdown');
    await app.flush();
    assert.deepEqual(app.calls.slice(-2), ['/steering?steering=-0.80', '/power?power=0.30']);
});

test('releasing either drive button stops power and retains steering', async () =>
{
    for (const id of ['forward', 'backward'])
    {
        const app = controller();
        await app.flush();
        app.element('steering').value = '-80';
        app.element('steering').emit('input');
        await app.flush();
        app.element(id).emit('pointerdown');
        await app.flush();
        app.calls.length = 0;
        app.element(id).emit('pointerup');
        await app.flush();
        assert.equal(app.state('direction'), 0);
        assert.equal(app.element('steering').value, '-80');
        assert.deepEqual(app.calls, ['/power?power=0', '/steering?steering=-0.80']);
        app.heartbeat();
        await app.flush();
        assert.equal(app.element('steering').value, '-80');
        app.element('center').emit('click');
        await app.flush();
        assert.equal(app.element('steering').value, '0');
        assert.deepEqual(app.calls.slice(-2), ['/power?power=0', '/steering?steering=0.00']);
    }
});

test('release during steering response never sends stale nonzero power', async () =>
{
    const app = controller();
    await app.flush();
    let resolve;
    app.fetchWith(url => url.startsWith('/steering')
        ? new Promise(r => {resolve = r;})
        : Promise.resolve({ok: true, text: async () => url}));
    app.calls.length = 0;
    app.element('forward').emit('pointerdown');
    app.element('forward').emit('pointerup');
    app.fetchWith(async url => ({ok: true, text: async () => url}));
    resolve({ok: true, text: async () => 'ok'});
    await app.flush();
    assert.ok(!app.calls.some(url => /power=0\.30/.test(url)));
    assert.ok(app.calls.includes('/power?power=0'));
});

test('HTTP failure cancels held control; reconnect sends only stop', async () =>
{
    const app = controller();
    await app.flush();
    app.fetchWith(async () => ({ok: false, status: 500}));
    app.element('forward').emit('pointerdown');
    await app.flush();
    assert.equal(app.state('direction'), 0);
    assert.equal(app.state('activePointer'), null);
    assert.equal(app.element('steering').value, '0');
    app.calls.length = 0;
    app.fetchWith(async url => ({ok: true, text: async () => url}));
    app.heartbeat();
    await app.flush();
    assert.deepEqual(app.calls, ['/power?power=0', '/steering?steering=0.00']);
});

test('blur and background clear motion; foreground resumes with stop', async () =>
{
    const app = controller();
    await app.flush();
    app.element('forward').emit('pointerdown');
    await app.flush();
    app.blur();
    await app.flush();
    assert.equal(app.state('direction'), 0);
    app.element('backward').emit('pointerdown');
    await app.flush();
    app.document.hidden = true;
    app.visibility();
    assert.equal(app.state('direction'), 0);
    app.calls.length = 0;
    app.heartbeat();
    await app.flush();
    assert.equal(app.calls.length, 0);
    app.document.hidden = false;
    app.visibility();
    await app.flush();
    assert.deepEqual(app.calls, ['/power?power=0', '/steering?steering=0.00']);
});
