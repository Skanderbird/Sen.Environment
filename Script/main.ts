namespace Sen.Script {
    /**
     * Console namespace of Script
     */

    export namespace Console {
        /**
         * --------------------------------------------------
         * JavaScript send output message
         * @param str - string to send
         * @param color - color to input
         * @returns The console output
         * --------------------------------------------------
         */

        export function send(str: any, color: Definition.Console.Color = Definition.Console.Color.DEFAULT): void {
            if (!Sen.Shell.is_gui) {
                Sen.Kernel.Console.print(`● ${str}`, color);
            } else {
                Sen.Kernel.Console.print(str, color);
            }
            return;
        }

        export function error(str: string): void {
            if (str !== undefined) {
                Console.send(str, Definition.Console.Color.RED);
            }
            return;
        }
    }

    /**
     * JavaScript Home
     */

    export namespace Home {
        /**
         * --------------------------------------------------
         * JS default path
         * --------------------------------------------------
         */

        export let participant: string = undefined!;

        /**
         * --------------------------------------------------
         * JavaScript setup current home
         * @returns Setup home
         * --------------------------------------------------
         */

        export function setup(): void {
            participant = Sen.Kernel.Path.dirname(Sen.Kernel.Home.script);
            return;
        }

        /**
         * --------------------------------------------------
         * JavaScript query current home
         * @param path - Path to query
         * @returns The resolved path
         * --------------------------------------------------
         */

        export function query(path: string): string {
            return Sen.Kernel.Path.resolve(path.replace(/^~(?=([/]|$))/gm, participant));
        }
    }

    /**
     * --------------------------------------------------
     * Script version
     * --------------------------------------------------
     */

    export const version = 1 as const;

    /**
     * --------------------------------------------------
     * Main function
     * @returns
     * --------------------------------------------------
     */

    export function main(): void {
        let result: string = launch();
        Console.error(result);
        return;
    }

    /**
     * --------------------------------------------------
     * Main thread
     * @returns
     * --------------------------------------------------
     */

    export function launch(): string {
        let result: string = undefined!;
        try {
            const before: number = Sen.Kernel.Thread.now();
            Sen.Script.Home.setup();
            Sen.Script.Module.load();
            Sen.Script.Setting.load();
            Sen.Script.Console.send(`Sen ~ Shell: ${Sen.Shell.version} & Kernel: ${Sen.Kernel.version} & Script: ${Sen.Script.version} ~ ${Sen.Kernel.OperatingSystem.current()} & ${Sen.Kernel.OperatingSystem.architecture()}`);
            Sen.Script.Console.send(Sen.Script.Setting.Language.get("js_has_been_loaded"), Sen.Script.Definition.Console.Color.GREEN);
            const after: number = Sen.Kernel.Thread.now();
            Sen.Script.Console.send(`${Sen.Script.Setting.Language.get("execution_time")}: ${(after - before).toFixed(3)}s`, Sen.Script.Definition.Console.Color.GREEN);
            throw new Error("test js error");
        } catch (e: unknown | any) {
            result = `${Sen.Script.Setting.Language.get("runtime_error")}: `;
            result += (e as Error).message;
            result += "\n";
            if ((e as Error).stack) {
                result += e.stack;
            }
        }
        return result;
    }

    /**
     * JavaScript Modules
     */

    export namespace Module {
        /**
         * --------------------------------------------------
         * All JS modules need to be initialized
         * @returns
         * --------------------------------------------------
         */

        export function load(): void {
            for (const script of script_list) {
                Sen.Kernel.JavaScript.evaluate_fs(Home.query(script));
            }
            return;
        }

        /**
         * Modules in queue await to be execute
         */

        export const script_list: Array<string> = [`~/Setting/Language/format.js`, `~/Setting/Setting.js`, `~/utility/definition.js`, `~/Support/Texture/Format.js`, `~/Support/PopCap/ResourceGroup/Convert.js`];
    }
}
