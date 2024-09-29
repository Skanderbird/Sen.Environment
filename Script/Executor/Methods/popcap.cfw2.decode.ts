namespace Sen.Script.Executor.Methods.PopCap.CFW2.Decode {
    /**
     * Argument for the current method
     */

    export interface Argument extends Sen.Script.Executor.Base {
        source: string;
        destination?: string;
    }

    /**
     * Argument for batch method
     */

    export interface BatchArgument extends Sen.Script.Executor.Base {
        directory: string;
    }

    /**
     * Async support
     */

    export interface AsyncArgument extends Sen.Script.Executor.Base {
        parameter: Array<[string, string]>;
    }

    /**
     * Configuration file if needed
     */

    export interface Configuration extends Sen.Script.Executor.Configuration {}

    /**
     * ----------------------------------------------
     * JavaScript forward method, this method need
     * to be evaluated during script loading time
     * ----------------------------------------------
     */

    export function forward(): void {
        Sen.Script.Executor.push_as_module<
            Sen.Script.Executor.Methods.PopCap.CFW2.Decode.Argument,
            Sen.Script.Executor.Methods.PopCap.CFW2.Decode.BatchArgument,
            Sen.Script.Executor.Methods.PopCap.CFW2.Decode.AsyncArgument,
            Sen.Script.Executor.Methods.PopCap.CFW2.Decode.Configuration
        >({
            id: "popcap.cfw2.decode",
            configuration_file: Home.query("~/Executor/Configuration/popcap.cfw2.decode.json"),
            direct_forward(argument: Argument): void {
                is_valid_source(argument, false);
                Console.obtained(argument.source);
                defined_or_default<Argument, string>(argument, "destination", `${Kernel.Path.except_extension(argument.source)}.json`);
                Console.output(argument.destination!);
                clock.start_safe();
                Kernel.Support.PopCap.CharacterFontWidget2.decode_fs(argument.source, argument.destination!);
                clock.stop_safe();
                return;
            },
            batch_forward(argument: BatchArgument): void {
                return basic_batch(this, argument, false);
            },
            is_enabled: true,
            configuration: undefined!,
            filter: ["file", /(.+)\.cfw2$/i],
        });
        return;
    }
}

Sen.Script.Executor.Methods.PopCap.CFW2.Decode.forward();
