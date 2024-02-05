namespace Sen.Script.Executor.Methods.JS.Evaluate {
    /**
     * Argument for the current method
     */

    export interface Argument extends Sen.Script.Executor.Base {
        source: string;
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

    export interface AsyncArgument<source extends string, destination extends string> extends Sen.Script.Executor.Base {
        parameter: Array<string>;
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
            Sen.Script.Executor.Methods.JS.Evaluate.Argument,
            Sen.Script.Executor.Methods.JS.Evaluate.BatchArgument,
            Sen.Script.Executor.Methods.JS.Evaluate.AsyncArgument<string, string>,
            Sen.Script.Executor.Methods.JS.Evaluate.Configuration
        >({
            id: "js.evaluate",
            configuration_file: Sen.Script.Home.query("~/Executor/Configuration/js.evaluate.json"),
            direct_forward(argument: Sen.Script.Executor.Methods.JS.Evaluate.Argument): void {
                Sen.Script.Executor.clock.start_safe();
                Sen.Script.Console.obtained(argument.source);
                const result: string | undefined = Sen.Kernel.JavaScript.evaluate_fs(argument.source) as unknown & string;
                Sen.Script.Console.display(Sen.Kernel.Language.get("js.process.done"), result, Sen.Script.Definition.Console.Color.GREEN);
                Sen.Script.Executor.clock.stop_safe();
                return;
            },
            batch_forward(argument: Sen.Script.Executor.Methods.JS.Evaluate.BatchArgument): void {
                const files: Array<string> = Sen.Kernel.FileSystem.read_directory(argument.directory).filter((path: string) => Sen.Kernel.FileSystem.is_file(path));
                files.forEach((source: string) => this.direct_forward({ source: source }));
                Sen.Script.Console.finished(Sen.Script.format(Sen.Kernel.Language.get("batch.process.count"), files.length));
                return;
            },
            async_forward(argument: Sen.Script.Executor.Methods.JS.Evaluate.AsyncArgument<string, string>): void {
                // to do
                Sen.Script.Console.finished(Sen.Script.format(Sen.Kernel.Language.get("batch.process.count"), argument.parameter.length));
                return;
            },
            is_enabled: false,
            configuration: undefined!,
            filter: ["file", /(.+).js$/gi],
        });
        return;
    }
}

Sen.Script.Executor.Methods.JS.Evaluate.forward();
