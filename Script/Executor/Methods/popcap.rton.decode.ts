namespace Sen.Script.Executor.Methods.PopCap.RTON.Decode {
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
            Sen.Script.Executor.Methods.PopCap.RTON.Decode.Argument,
            Sen.Script.Executor.Methods.PopCap.RTON.Decode.BatchArgument,
            Sen.Script.Executor.Methods.PopCap.RTON.Decode.AsyncArgument,
            Sen.Script.Executor.Methods.PopCap.RTON.Decode.Configuration
        >({
            id: "popcap.rton.decode",
            configuration_file: Sen.Script.Home.query("~/Executor/Configuration/popcap.rton.decode.json"),
            direct_forward(argument: Sen.Script.Executor.Methods.PopCap.RTON.Decode.Argument): void {
                Sen.Script.Console.obtained(argument.source);
                defined_or_default<Sen.Script.Executor.Methods.PopCap.RTON.Decode.Argument, string>(argument, "destination", `${Sen.Kernel.Path.except_extension(argument.source)}.json`);
                Sen.Script.Console.output(argument.destination!);
                Sen.Script.Executor.clock.start_safe();
                Sen.Kernel.Support.PopCap.RTON.decode_fs(argument.source, argument.destination!);
                Sen.Script.Executor.clock.stop_safe();
                return;
            },
            batch_forward(argument: Sen.Script.Executor.Methods.PopCap.RTON.Decode.BatchArgument): void {
                return basic_batch(this, argument, false);
            },
            is_enabled: true,
            configuration: undefined!,
            filter: ["file", /(.+)(\.rton|pp\.dat)$/i],
        });
        return;
    }
}

Sen.Script.Executor.Methods.PopCap.RTON.Decode.forward();
