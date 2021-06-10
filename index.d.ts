/**
 * A class for creating shared memory blocks
 */
export default class shared_memory {
    /**
     * The size of the memory block in bytes
     */
    public readonly size: number;

    /**
     * The name of the memory block
     */
    public readonly name: string;

    /**
     * Whether this is the instance that created the block
     */
    public readonly host: boolean;

    /**
     * Whether the memory block can be read globally
     */
    public readonly global: boolean;

    /**
     * Create a new shared memory block
     *
     * @param name the name of the block
     * @param sizeInBytes the size of the block in bytes
     * @param global whether the memory can be read globally. Defaults to false.
     * @param host whether to create the memory block. Defaults to true.
     */
    public constructor(name: string, sizeInBytes: number, global?: boolean, host?: boolean);

    /**
     * Set the data
     *
     * @param data the data to write
     */
    public set data(data: string);

    /**
     * Get the data from the memory block.
     * Basically an alias for {@link read}.
     */
    public get data(): string;

    /**
     * Set a data buffer
     *
     * @param data the data to write
     */
    public set buffer(data: Buffer);

    /**
     * Get the data as a buffer from the memory.
     * Basically an alias for {@link readBuffer}.
     */
    public get buffer(): Buffer;

    /**
     * Write some data to the memory
     *
     * @param data the data to write
     */
    public write(data: string | Buffer): void;

    /**
     * Read a string from the memory.
     * Reads min(strlen(buffer), buffer.length) characters.
     */
    public read(): string;

    /**
     * Read a the data into a buffer.
     * Always reads buffer.length bytes into the buffer.
     */
    public readBuffer(): Buffer;
}