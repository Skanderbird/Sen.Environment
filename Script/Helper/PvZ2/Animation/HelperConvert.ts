namespace Sen.Script.Support.PopCap.Animation.Miscellaenous.HelperConvert {
	export type SquareHelper = [bigint, bigint];

	export interface AnimationImageHelper {
		name: string;
		size: SquareHelper;
		transform: Array<number>;
	}

	export type AnimationCommandHelper = [string, string];

	export interface AnimationRemoveHelper {
		index: bigint;
	}

	export interface AnimationFrameHelper {
		label: string;
		stop: boolean;
		command: Array<AnimationCommandHelper>;
		remove: Array<AnimationRemoveHelper>;
		append: Array<Animation.AnimationAppend>;
		change: Array<Animation.AnimationMove>;
	}

	export interface AnimationSpriteHelper {
		name: string;
		work_area: SquareHelper;
		frame: Array<AnimationFrameHelper>;
	}

	export interface SexyAnimationHelper {
		version: bigint;
		frame_rate: bigint;
		position: SquareHelper;
		size: SquareHelper;
		image: Array<AnimationImageHelper>;
		sprite: Array<AnimationSpriteHelper>;
		main_sprite: AnimationSpriteHelper;
	}

	export function convert(animation: SexyAnimation): SexyAnimationHelper {
		const rewrite_sprite = (e: Animation.AnimationSprite) => {
			const sprite: AnimationSpriteHelper = {
				name: e.name,
				work_area: [e.work_area.start, e.work_area.duration],
				frame: e.frame.map(function (f) {
					const frame_helper: AnimationFrameHelper = {
						label: f.label,
						stop: f.stop,
						command: f.command.map(function (c) {
							const command_helper: AnimationCommandHelper = [c.command, c.parameter];
							return command_helper;
						}),
						remove: f.remove.map(function (r) {
							return { index: r };
						}),
						append: f.append,
						change: f.change,
					};
					return frame_helper;
				}),
			};
			return sprite;
		};

		const animation_helper: SexyAnimationHelper = {
			version: animation.version,
			frame_rate: animation.frame_rate,
			position: [BigInt(animation.position.x), BigInt(animation.position.y)],
			size: [BigInt(animation.size.width), BigInt(animation.size.height)],
			image: animation.image.map(function (e) {
				const image_helper: AnimationImageHelper = {
					name: `${e.path}|${e.id}`,
					size: [BigInt(animation.size.width), BigInt(animation.size.height)],
					transform: e.transform,
				};
				return image_helper;
			}),
			sprite: animation.sprite.map((e) => rewrite_sprite(e)),
			main_sprite: rewrite_sprite(animation.main_sprite),
		};

		return animation_helper;
	}

	export function process(animation: SexyAnimation): SexyAnimationHelper {
		const animation_helper = convert(animation);
		return animation_helper;
	}

	export function execute(): void {
		const source = Console.path(
			Kernel.Language.get(
				'script.helper.pvz2.animation.helper_convert.input_source_pam_json',
			),
			'file',
		);
		const animation: SexyAnimation = Kernel.JSON.deserialize_fs<SexyAnimation>(source);
		Kernel.JSON.serialize_fs(
			`${Kernel.Path.except_extension(source)}.twinstar.json`,
			process(animation),
		);
	}
}

Sen.Script.Support.PopCap.Animation.Miscellaenous.HelperConvert.execute();
