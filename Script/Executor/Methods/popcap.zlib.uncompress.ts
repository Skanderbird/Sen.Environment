namespace Sen.Script.Executor.Methods.PopCap.Zlib.Uncompress {
    /**
     * Argument for the current method
     */

    export interface Argument extends Sen.Script.Executor.Base {
        source: string;
        destination?: string;
        key?: string;
        iv?: string;
        use_64_bit_variant?: boolean;
    }

    /**
     * Argument for batch method
     */

    export interface BatchArgument extends Sen.Script.Executor.Base {
        directory: string;
        key?: string;
        iv?: string;
        use_64_bit_variant?: boolean;
    }

    /**
     * Configuration file if needed
     */

    export interface Configuration extends Sen.Script.Executor.Configuration {
        key?: string;
        iv?: string;
        use_64_bit_variant?: boolean;
    }

    /**
     * ----------------------------------------------
     * JavaScript forward method, this method need
     * to be evaluated during script loading time
     * ----------------------------------------------
     */

    export function forward(): void {
        Sen.Script.Executor.push_as_module<
            Sen.Script.Executor.Methods.PopCap.Zlib.Uncompress.Argument,
            Sen.Script.Executor.Methods.PopCap.Zlib.Uncompress.BatchArgument,
            Sen.Script.Executor.Methods.PopCap.Zlib.Uncompress.Configuration
        >({
            id: "popcap.zlib.uncompress",
            configuration_file: Home.query("~/Executor/Configuration/popcap.zlib.uncompress.json"),
            direct_forward(argument: Argument): void {
                is_valid_source(argument, false);
                Console.obtained(argument.source);
                defined_or_default<Argument, string>(argument, "destination", `${argument.source}.bin`);
                Console.output(argument.destination!);
                load_boolean(argument, "use_64_bit_variant", this.configuration, Kernel.Language.get("popcap.zlib.uncompress.use_64_bit_variant"));
                clock.start_safe();
                Kernel.Support.PopCap.Zlib.uncompress_fs(argument.source, argument.destination!, argument.use_64_bit_variant!);
                clock.stop_safe();
                return;
            },
            batch_forward(argument: BatchArgument): void {
                return basic_batch(this, argument, false);
            },
            is_enabled: true,
            configuration: undefined!,
            filter: ["file", /(.+)\.(compiled|smf|bin)$/i],
            option: 72n,
        });
        return;
    }
}

Sen.Script.Executor.Methods.PopCap.Zlib.Uncompress.forward();
