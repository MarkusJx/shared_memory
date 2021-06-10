/**
 * A class for creating shared memory blocks.
 *
 * Note on global properties:
 * The global property only have an effect on windows
 * machines, when running on any other machine, the passed
 * value will be ignored. If true is passed as an argument,
 * the shared memory block will be accessible globally.
 * In that case, the program must be run with administrator rights.
 * On linux, any program creating a shared memory block must be
 * executed with root rights.
 * @see https://docs.microsoft.com/en-us/windows/win32/termserv/kernel-object-namespaces
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
     * Generate a shared memory block id
     *
     * @param global whether the memory can be read globally. Defaults to false.
     * @return the generated id
     */
    public static generateId(global?: boolean): string;

    /**
     * Generate a shared memory block id
     *
     * @param global whether the memory can be read globally. Defaults to false.
     * @return the generated id
     */
    public static generateIdAsync(global?: boolean): Promise<string>;

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
     *
     * @return the read data
     */
    public read(): string;

    /**
     * Read a the data into a buffer.
     * Always reads buffer.length bytes into the buffer.
     *
     * @return the read data
     */
    public readBuffer(): Buffer;
}