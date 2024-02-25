namespace Sen.Script.Support.PopCap.ResourceStreamBundle.Miscellaneous.Obfuscate {
    /**
     * Version
     */

    export const versions: Array<bigint> = [3n, 4n];

    /**
     * Head
     */

    export interface RSBHead {
        file_offset: bigint;
        file_list_length: bigint;
        file_list_begin: bigint;
        rsg_list_length: bigint;
        rsg_list_begin: bigint;
        rsg_number: bigint;
        rsg_info_begin: bigint;
        rsg_info_each_length: bigint;
        composite_number: bigint;
        composite_info_begin: bigint;
        composite_info_each_length: bigint;
        composite_list_length: bigint;
        composite_list_begin: bigint;
        autopool_number: bigint;
        autopool_info_begin: bigint;
        autopool_info_each_length: bigint;
        ptx_number: bigint;
        ptx_info_begin: bigint;
        ptx_info_each_length: bigint;
    }

    // -----------------------------------------------------------------

    export function read_head(sen: Kernel.DataStreamView) {
        sen.read_position = 16n;
        const file_list_length = sen.readUint32();
        const file_list_begin = sen.readUint32();
        sen.read_position += 8n;
        const rsg_list_length = sen.readUint32();
        const rsg_list_begin = sen.readUint32();
        const rsg_number = sen.readUint32();
        const rsg_info_begin = sen.readUint32();
        const rsg_info_each_length = sen.readUint32();
        const composite_number = sen.readUint32();
        const composite_info_begin = sen.readUint32();
        const composite_info_each_length = sen.readUint32();
        const composite_list_length = sen.readUint32();
        const composite_list_begin = sen.readUint32();
        const autopool_number = sen.readUint32();
        const autopool_info_begin = sen.readUint32();
        const autopool_info_each_length = sen.readUint32();
        const ptx_number = sen.readUint32();
        const ptx_info_begin = sen.readUint32();
        const ptx_info_each_length = sen.readUint32();
        sen.read_position += 12n;
        const file_offset = sen.readUint32();
        return {
            file_offset,
            file_list_length,
            file_list_begin,
            rsg_list_length,
            rsg_list_begin,
            rsg_number,
            rsg_info_begin,
            rsg_info_each_length,
            composite_number,
            composite_info_begin,
            composite_info_each_length,
            composite_list_length,
            composite_list_begin,
            autopool_number,
            autopool_info_begin,
            autopool_info_each_length,
            ptx_number,
            ptx_info_begin,
            ptx_info_each_length,
        };
    }

    // -----------------------------------------------------------------

    export function make_random(): bigint {
        let result: bigint = undefined!;
        while (true) {
            result = BigInt(Math.floor(Math.random() * 0xff));
            if (!versions.includes(result)) {
                break;
            }
        }
        return result;
    }

    // -----------------------------------------------------------------

    export function disturb_header(sen: Kernel.DataStreamView): void {
        sen.write_position = 4n;
        sen.writeUint8(make_random());
        return;
    }

    // -----------------------------------------------------------------

    export function process(sen: Kernel.DataStreamView): void {
        const rsb_head_info = read_head(sen);
        disturb_header(sen);
        sen.read_position = rsb_head_info.rsg_info_begin;
        for (let i = 0n; i < rsb_head_info.rsg_number; ++i) {
            const start_index = sen.read_position;
            const autopoolstart_index = rsb_head_info.autopool_info_begin + i * 152n;
            sen.writeNull(128n, start_index);
            sen.writeNull(128n, autopoolstart_index);
            sen.writeNull(4n, start_index + 132n);
            const packetOffset = sen.readUint32(start_index + 128n);
            sen.writeNull(64n, packetOffset);
            sen.read_position = start_index + rsb_head_info.rsg_info_each_length;
        }
        return;
    }

    // -----------------------------------------------------------------

    export function process_fs(source: string, destination: string): void {
        const rsb = new Kernel.DataStreamView(source);
        process(rsb);
        rsb.out_file(destination);
        return;
    }
}
