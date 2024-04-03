namespace Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt {
    /**
     * Argument for the current method
     */

    export interface Argument extends Sen.Script.Executor.Base {
        source: string;
        destination?: string;
        key?: string;
    }

    /**
     * Argument for batch method
     */

    export interface BatchArgument extends Sen.Script.Executor.Base {
        directory: string;
        key?: string;
    }

    /**
     * Async support
     */

    export interface AsyncArgument extends Sen.Script.Executor.Base {
        parameter: Array<[string, string]>;
        key?: string;
    }

    /**
     * Configuration file if needed
     */

    export interface Configuration extends Sen.Script.Executor.Configuration {
        key: "?" | string;
    }

    /**
     * ----------------------------------------------
     * JavaScript forward method, this method need
     * to be evaluated during script loading time
     * ----------------------------------------------
     */

    export function forward(): void {
        Sen.Script.Executor.push_as_module<
            Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.Argument,
            Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.BatchArgument,
            Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.AsyncArgument,
            Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.Configuration
        >({
            id: "popcap.crypt_data.encrypt",
            configuration_file: Sen.Script.Home.query("~/Executor/Configuration/popcap.crypt_data.encrypt.json"),
            direct_forward(argument: Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.Argument): void {
                Sen.Script.Console.obtained(argument.source);
                defined_or_default<Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.Argument, string>(argument, "destination", `${Sen.Kernel.Path.except_extension(argument.source)}.bin`);
                Sen.Script.Console.output(argument.destination!);
                Sen.Script.Executor.load_string(argument, "key", this.configuration, Sen.Kernel.Language.get("popcap.crypt_data.encrypt.key"));
                Sen.Script.Executor.clock.start_safe();
                Sen.Kernel.Support.PopCap.CryptData.encrypt_fs(argument.source, argument.destination!, argument.key!);
                Sen.Script.Executor.clock.stop_safe();
                return;
            },
            batch_forward(argument: Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.BatchArgument): void {
                return basic_batch(this, argument, false);
            },
            is_enabled: true,
            configuration: undefined!,
            filter: ["file", /(.+)\.bin$/i],
        });
        return;
    }
}

Sen.Script.Executor.Methods.PopCap.CryptData.Encrypt.forward();
