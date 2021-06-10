const {shared_memory} = require('./build/Debug/shared_memory');

console.log(shared_memory);
let shared = new shared_memory("MyFileMappingObject", 1024);

console.log(shared);
console.log(shared.size);

setTimeout(() => {
    shared.data = "abc";
    console.log(shared.data);

    shared.buffer = Buffer.alloc(1024, 1);
    console.log(shared.buffer)
}, 100);