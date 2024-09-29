namespace Sen.Script.Executor.Methods.PopCap.Particles.Decode {
    // Using platform

    export type Platform = Kernel.Support.PopCap.Particles.Platform;

    /**
     * Argument for the current method
     */

    export interface Argument extends Sen.Script.Executor.Base {
        source: string;
        destination?: string;
        platform?: Platform;
    }

    /**
     * Argument for batch method
     */

    export interface BatchArgument extends Sen.Script.Executor.Base {
        directory: string;
        platform?: Platform;
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
     * Detail namespace
     */

    export namespace Detail {
        /**
         * Platform supported
         */

        export const _platform: Array<Kernel.Support.PopCap.Particles.Platform> = ["pc", "game-console", "phone-32", "phone-64", "tv"];
        /**
         *
         * Typical Style
         *
         */

        export function platform(): Array<[bigint, string, string]> {
            return _platform.map((e, i) => [BigInt(i + 1), e as string, Kernel.Language.get(`popcap.reanim.platform.${e}`)]);
        }
    }
    /**
     * ----------------------------------------------
     * JavaScript forward method, this method need
     * to be evaluated during script loading time
     * ----------------------------------------------
     */

    export function forward(): void {
        Sen.Script.Executor.push_as_module<
            Sen.Script.Executor.Methods.PopCap.Particles.Decode.Argument,
            Sen.Script.Executor.Methods.PopCap.Particles.Decode.BatchArgument,
            Sen.Script.Executor.Methods.PopCap.Particles.Decode.AsyncArgument,
            Sen.Script.Executor.Methods.PopCap.Particles.Decode.Configuration
        >({
            id: "popcap.particles.decode",
            configuration_file: Home.query("~/Executor/Configuration/popcap.particles.decode.json"),
            direct_forward(argument: Argument): void {
                is_valid_source(argument, false);
                Console.obtained(argument.source);
                defined_or_default<Argument, string>(argument, "destination", `${Kernel.Path.except_extension(argument.source)}.json`);
                Console.output(argument.destination!);
                Console.argument(Kernel.Language.get("popcap.particles.decode.generic"));
                configurate_or_input(argument, "platform", Detail.platform());
                clock.start_safe();
                Kernel.Support.PopCap.Particles.decode_fs(argument.source, argument.destination!, argument.platform!);
                clock.stop_safe();
                return;
            },
            batch_forward(argument: BatchArgument): void {
                return basic_batch(this, argument, false);
            },
            is_enabled: true,
            configuration: undefined!,
            filter: ["file", /(.+)(\.xml|\.xml\.compiled)$/i],
        });
        return;
    }
}

Sen.Script.Executor.Methods.PopCap.Particles.Decode.forward();
