const assert = require("assert");
const crypto = require("crypto");
//const {shared_memory} = require('./build/Debug/shared_memory');
const shared_memory = require('./index');

const rand = () => Math.floor(Math.random() * 512);
const randBuffer = () => crypto.randomBytes(rand());
const randString = () => randBuffer().toString('hex');
const fixedRandBuffer = () => crypto.randomBytes(1024);

const NUM_RUNS = 1000;

describe('shared_memory', () => {
    describe('local read/write', function () {
        let mem;
        it('create', () => {
            mem = new shared_memory("SomeMemory", 1024);
            assert.strictEqual(mem.size, 1024);
        });

        it('string setter/getter', () => {
            for (let i = 0; i < NUM_RUNS; i++) {
                const dt = randString();
                mem.data = dt;

                assert.strictEqual(mem.data, dt);
                assert.strictEqual(mem.read(), dt);
            }
        });

        it('string write', () => {
            for (let i = 0; i < NUM_RUNS; i++) {
                const dt = randString();
                mem.write(dt);

                assert.strictEqual(mem.data, dt);
                assert.strictEqual(mem.read(), dt);
            }
        });

        it('buffer setter/getter', () => {
            for (let i = 0; i < NUM_RUNS; i++) {
                const dt = fixedRandBuffer();
                mem.buffer = dt;

                assert.strictEqual(dt.equals(mem.buffer), true);
                assert.strictEqual(dt.equals(mem.readBuffer()), true);
            }
        });

        it('buffer write', () => {
            for (let i = 0; i < NUM_RUNS; i++) {
                const dt = fixedRandBuffer();
                mem.write(dt);

                assert.strictEqual(dt.equals(mem.buffer), true);
                assert.strictEqual(dt.equals(mem.readBuffer()), true);
            }
        });
    });

    describe('shared read/write', function () {
        let host, client;
        it('create host', () => {
            host = new shared_memory("SomeMemory", 1024, false, true);
            assert.strictEqual(host.size, 1024);
        });

        it('create client', () => {
            client = new shared_memory("SomeMemory", 1024, false, false);
            assert.strictEqual(client.size, 1024);
        });

        describe('server write', () => {
            it('string setter/getter', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = randString();
                    host.data = dt;

                    assert.strictEqual(client.data, dt);
                    assert.strictEqual(client.read(), dt);
                }
            });

            it('string write', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = randString();
                    host.write(dt);

                    assert.strictEqual(client.data, dt);
                    assert.strictEqual(client.read(), dt);
                }
            });

            it('buffer setter/getter', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = fixedRandBuffer();
                    host.buffer = dt;

                    assert.strictEqual(dt.equals(client.buffer), true);
                    assert.strictEqual(dt.equals(client.readBuffer()), true);
                }
            });

            it('buffer write', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = fixedRandBuffer();
                    host.write(dt);

                    assert.strictEqual(dt.equals(client.buffer), true);
                    assert.strictEqual(dt.equals(client.readBuffer()), true);
                }
            });
        });

        describe('client write', () => {
            it('string setter/getter', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = randString();
                    client.data = dt;

                    assert.strictEqual(host.data, dt);
                    assert.strictEqual(host.read(), dt);
                }
            });

            it('string write', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = randString();
                    client.write(dt);

                    assert.strictEqual(host.data, dt);
                    assert.strictEqual(host.read(), dt);
                }
            });

            it('buffer setter/getter', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = fixedRandBuffer();
                    client.buffer = dt;

                    assert.strictEqual(dt.equals(host.buffer), true);
                    assert.strictEqual(dt.equals(host.readBuffer()), true);
                }
            });

            it('buffer write', () => {
                for (let i = 0; i < NUM_RUNS; i++) {
                    const dt = fixedRandBuffer();
                    client.write(dt);

                    assert.strictEqual(dt.equals(host.buffer), true);
                    assert.strictEqual(dt.equals(host.readBuffer()), true);
                }
            });
        });
    });
});
