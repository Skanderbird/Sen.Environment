namespace Sen.Script.Helper.PvZ2.Animation.GenerateAtlasFromData {
	export const z_path: Array<string> = ['images', '1536', 'full', 'worldmap', 'lostcity'];

	export const allowance = ['1536', '768', '384', '640', '1200'] as const;

	export function process(
		atlas: Support.PopCap.Atlas.Structure.Definition,
		record: Support.PopCap.Animation.Miscellaenous.GenerateData.Data,
	): void {
		const paths: Array<string> = Object.keys(record.image);
		paths.forEach((path: string) => {
			atlas.groups[record.image[path].id] = {
				default: {
					x: 0n,
					y: 0n,
				},
				path: [...z_path, path].join('/'),
			};
		});
	}

	export function get_resolution(): Array<string> {
		return allowance.filter((e: string) => z_path.find((z: string) => z === e));
	}

	export function process_fs(): void {
		const record = Console.path(
			Kernel.Language.get(
				'script.helper.pvz2.animation.generate_atlas_from_data.input_data_json',
			),
			'file',
		);
		Console.display(
			Kernel.Language.get(
				'script.helper.pvz2.animation.generate_atlas_from_data.input_subgroup_name',
			),
			'cyan',
		);
		const subgroup = readline();
		const resolutions = get_resolution();
		assert(
			resolutions.length > 0,
			format(
				Kernel.Language.get(
					'script.helper.pvz2.animation.generate_atlas_from_data.resolution_not_found',
				),
				allowance,
			),
		);
		const destination = Kernel.Path.dirname(record);
		resolutions.forEach(function handle_resolution(resolution) {
			const atlas: Support.PopCap.Atlas.Structure.Definition = {
				expand_path: 'array',
				method: 'id',
				trim: false,
				subgroup: `${subgroup}_${resolution}`,
				res: resolution,
				groups: {},
			};
			process(
				atlas,
				Kernel.JSON.deserialize_fs<Support.PopCap.Animation.Miscellaenous.GenerateData.Data>(
					record,
				),
			);
			Kernel.JSON.serialize_fs(`${destination}/${subgroup}_${resolution}.json`, atlas);
		});
	}
}
Sen.Script.Helper.PvZ2.Animation.GenerateAtlasFromData.process_fs();
