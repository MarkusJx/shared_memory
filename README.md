[![shared_memory](https://socialify.git.ci/MarkusJx/shared_memory/image?description=1&language=1&owner=1&theme=Light)](https://github.com/MarkusJx/shared_memory#readme)

## Installation
```sh
npm install @markusjx/shared_memory
```
*Requires Node.js 12 or later*

## Note on any ``global`` properties
The global property only have an effect on Windows
machines, when running on any other machine, the passed
value will be ignored. When passing true as an argument,
the shared memory block will be accessible globally.
In that case, the program must be run with administrator rights.
On linux, any program creating a shared memory block must be executed with root rights.

Read more [here](https://docs.microsoft.com/en-us/windows/win32/termserv/kernel-object-namespaces).

## Usage
### Import the module
```js
const shared_memory = require('@markusjx/shared_memory');
```

### Class ``shared_memory``
#### ``shared_memory.generateId``
Generate an id for a shared memory block.

**Arguments:**
* ``global?: boolean`` - Whether the memory block can be read globally. Defaults to ``false``.

**Returns**

``string`` - The generated id

#### ``shared_memory.generateIdAsync``
Generate an id for a shared memory block. Async call.

**Arguments**
* ``global?: boolean`` - Whether the memory block can be read globally. Defaults to ``false``.

**Returns**

``Promise<string>`` - The generated id

#### ``new shared_memory``
Create a new shared memory block.

**Arguments**
* ``name: string`` - The name (id) of the memory block
* ``sizeInBytes: number`` - The size of the memory block in bytes
* ``global?: boolean`` - Whether the memory block can be read globally. Defaults to ``false``.
* ``host?: boolean`` - Whether to create the memory block. Defaults to ``true``.

**Throws**

``Error`` - If a memory block with the given id already exists (if ``host`` is set to ``true``), or the block couldn't be attached.

#### ``shared_memory.set data``
Copy a string to the shared memory block.

**Arguments**
* ``data: string`` - The data to copy

**Throws**

``Error`` - If the string is too big for the buffer

#### ``shared_memory.get data``
Get a string from the memory block. Reads ``std::max(strlen(buffer), sizeof(buffer))`` bytes from the buffer.
The string length therefore is determined by either a delimiting ``\0`` or the length of the buffer.

**Returns**

``string`` - The read data

#### ``shared_memory.set buffer``
Copy data from a node ``Buffer`` to the memory block.

**Arguments**
* ``data: Buffer`` - The data to copy

**Throws**

``Error`` - If the Buffer is larger than the shared memory block

#### ``shared_memory.get buffer``
Read data from the memory block into a node ``Buffer``.
Always reads ``sizeof(buffer)`` into the result ``Buffer``.

**Returns**

``Buffer`` - The read data

#### ``shared_memory.write``
Write some data to the buffer. Writes either a ``string`` or a node ``Buffer``.

**Arguments**
* ``data: string | Buffer`` - The data to copy

**Returns**

``void``

**Throws**

``Error`` - If the shared memory block is too small for the data to write

#### ``shared_memory.read``
Read a string from the memory. Reads ``std::max(strlen(buffer), sizeof(buffer))`` bytes from the buffer.
The string length therefore is determined by either a delimiting ``\0`` or the length of the buffer.

**Returns**

``string`` - the read data

#### ``shared_memory.readBuffer``
Read data from the memory block into a node ``Buffer``.
Always reads ``sizeof(buffer)`` into the result ``Buffer``.

**Returns**

``Buffer`` - The read data

## Example
Host code:
```ts
const shared_memory = require('@markusjx/shared_memory');

// Generate a memory id
const id = shared_memory.generateId();

// Create a new shared memory block
// with a size of 1024 bytes
const memory = new shared_memory(id, 1024, false, true);

// Write data to the memory
memory.data = "Some data";

// Signal the client that the data is ready
// Wait until the client has read all the data

// Write some new data in form of a buffer
memory.buffer = Buffer.from("Some data");

// Signal the client that the data is ready
```

Client code:

```ts
import shared_memory from "./index";

// Attach the created memory
const memory = new shared_memory(id, 1024, false, false);

// Wait until the data is ready

// Read the data 
const read = memory.data;

// Signal the host that the data has been read
// Wait until the next data is ready

// Read the data into a buffer.
// This will contain all of the data in the memory block.
const buf = memory.buffer;
```
