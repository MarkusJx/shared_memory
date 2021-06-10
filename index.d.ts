export default class shared_memory {
    public readonly size: number;
    public readonly name: string;
    public readonly host: boolean;
    public readonly global: boolean;

    public constructor(name: string, sizeInBytes: number, global?: boolean, host?: boolean);

    public set data(data: string);

    public get data(): string;

    public set buffer(data: Buffer);

    public get buffer(): Buffer;

    public writeSync(data: string | Buffer): void;

    public readSync(): string;

    public readBufferSync(): Buffer;
}