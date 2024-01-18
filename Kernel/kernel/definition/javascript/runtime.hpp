#pragma once

#include "kernel/definition/utility.hpp"
#include "kernel/definition/javascript/converter.hpp"

namespace Sen::Kernel::Definition::JavaScript
{

	namespace FileSystem = Sen::Kernel::FileSystem;

	/**
	 * Template
	*/

	template <typename Type>
	concept SpaceX = std::is_same_v<Type, int32_t> || std::is_same_v<Type, uint32_t>
	|| std::is_same_v<Type, int64_t> || std::is_same_v<Type, uint64_t> || std::is_same_v<Type, float> || std::is_same_v<Type, double> || std::is_same_v<Type, std::string> || std::is_same_v<Type, bool>;

	/**
	 * JS Runtime Native Handler
	*/
	struct Runtime {

		protected:
			using JS = Runtime;

		private:
			/**
			 * JS Runtime
			*/

			JSRuntime *rt = JS_NewRuntime();

			/**
			 * JS Context
			*/

			JSContext* ctx = JS_NewContext(thiz.rt);

			/**
			 * Free JS Value
			*/

			inline auto free_value(
				const JSValue & that
			) -> void
			{
				JS_FreeValue(thiz.ctx, that);
				return;
			}

			/**
			 * Free C String
			*/

			inline auto free_string(
				const char* that
			) -> void
			{
				JS_FreeCString(thiz.ctx, that);
				return;
			}

		public:

			/**
			 * JS Handler Constructor
			*/

			explicit Runtime(

			)
			{

			}

			/**
			 * JS Exception
			*/
			inline auto exception(
				const JSValue & val
			) -> std::string
			{
				auto result = std::string{};
				auto exception = JS_GetException(thiz.ctx);
				auto exception_stack = JS_ToCString(thiz.ctx, exception);
				result += std::string{exception_stack};
				if(JS_IsError(thiz.ctx, exception)){
					auto js_stack = JS_GetPropertyStr(thiz.ctx, exception, "stack");
					if(JS::not_undefined(js_stack)){
						auto js_exception = JS_ToCString(thiz.ctx, js_stack);
						result += std::string{js_exception};
						thiz.free_string(js_exception);
					}
					thiz.free_value(js_stack);
				}
				thiz.free_string(exception_stack);
				thiz.free_value(exception);
				return result;
			}

			#pragma region register object

			/**
			 * Use this to register an instance of JS Object
			*/

			inline auto register_object(
				std::function<void (JSRuntime*, JSContext*)> register_method
			) -> void
			{
				register_method(thiz.rt, thiz.ctx);
				return;
			}

			/**
			 * Use this to register an instance of JS Object
			*/

			inline auto register_object(
				std::function<void (JSContext*)> register_method
			) -> void
			{
				register_method(thiz.ctx);
				return;
			}

			#pragma endregion


			#pragma region comment
			/**
			 * --------------------------------
			 * Evaluate JS through file reading
			 * @param source: JS source
			 * @return: JS Value after evaluate
			 * --------------------------------
			*/

			#pragma endregion

			inline auto evaluate_fs(
				std::string_view source
			) -> JSValue
			{
				return thiz.evaluate(FileSystem::read_file(source), source);
			}

			/**
			 * Check if an JSValue is undefined
			*/

			inline static auto constexpr not_undefined(
				const JSValue & that
			) -> bool
			{
				return JS_VALUE_GET_TAG(that) != JS_TAG_UNDEFINED;
			}

			/**
			 * Check if an JSValue is null
			*/

			inline static auto constexpr not_null(
				const JSValue & that
			) -> bool
			{
				return JS_VALUE_GET_TAG(that) != JS_TAG_NULL;
			}

			/**
			 * ------------------------------------------------------------
			 * Evaluate JavaScript
			 * @param source_data: the eval script data
			 * @param source_file: the source file contains the script data
			 * @return: JS eval data
			 * ------------------------------------------------------------
			*/

			inline auto evaluate(
				std::string_view source_data,
				std::string_view source_file
			) -> JSValue
			{
				auto eval_result = JS_Eval(thiz.ctx, source_data.data(), source_data.size(), source_file.data(), JS_EVAL_TYPE_GLOBAL);
				if(JS_IsException(eval_result)){
					throw Exception(thiz.exception(eval_result));
				}
				return eval_result;
			}

			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				JS_SetPropertyStr(ctx, global_obj, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param object_name: Object name
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view object_name,
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}


			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), innerObject);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}


			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @param obj3_name: Object 3 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto middleObject = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(middleObject)) {
					middleObject = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, middleObject, obj3_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, middleObject, obj3_name.data(), innerObject);
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), middleObject);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @param obj3_name: Object 3 wrapper
			 * @param obj4_name: Object 4 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto middle1Object = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(middle1Object)) {
					middle1Object = JS_NewObject(ctx);
				}
				auto middle2Object = JS_GetPropertyStr(ctx, middle1Object, obj3_name.data());
				if (JS_IsUndefined(middle2Object)) {
					middle2Object = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, middle2Object, obj4_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, middle2Object, obj4_name.data(), innerObject);
				JS_SetPropertyStr(ctx, middle1Object, obj3_name.data(), middle2Object);
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), middle1Object);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @param obj3_name: Object 3 wrapper
			 * @param obj4_name: Object 4 wrapper
			 * @param obj5_name: Object 5 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto middle1Object = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(middle1Object)) {
					middle1Object = JS_NewObject(ctx);
				}
				auto middle2Object = JS_GetPropertyStr(ctx, middle1Object, obj3_name.data());
				if (JS_IsUndefined(middle2Object)) {
					middle2Object = JS_NewObject(ctx);
				}
				auto middle3Object = JS_GetPropertyStr(ctx, middle2Object, obj4_name.data());
				if (JS_IsUndefined(middle3Object)) {
					middle3Object = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, middle3Object, obj5_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, middle3Object, obj5_name.data(), innerObject);
				JS_SetPropertyStr(ctx, middle2Object, obj4_name.data(), middle3Object);
				JS_SetPropertyStr(ctx, middle1Object, obj3_name.data(), middle2Object);
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), middle1Object);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}


			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @param obj3_name: Object 3 wrapper
			 * @param obj4_name: Object 4 wrapper
			 * @param obj5_name: Object 5 wrapper
			 * @param obj6_name: Object 6 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view function_name
			) const -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto middle1Object = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(middle1Object)) {
					middle1Object = JS_NewObject(ctx);
				}
				auto middle2Object = JS_GetPropertyStr(ctx, middle1Object, obj3_name.data());
				if (JS_IsUndefined(middle2Object)) {
					middle2Object = JS_NewObject(ctx);
				}
				auto middle3Object = JS_GetPropertyStr(ctx, middle2Object, obj4_name.data());
				if (JS_IsUndefined(middle3Object)) {
					middle3Object = JS_NewObject(ctx);
				}
				auto middle4Object = JS_GetPropertyStr(ctx, middle3Object, obj5_name.data());
				if (JS_IsUndefined(middle4Object)) {
					middle4Object = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, middle4Object, obj6_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, middle4Object, obj6_name.data(), innerObject);
				JS_SetPropertyStr(ctx, middle3Object, obj5_name.data(), middle4Object);
				JS_SetPropertyStr(ctx, middle2Object, obj4_name.data(), middle3Object);
				JS_SetPropertyStr(ctx, middle1Object, obj3_name.data(), middle2Object);
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), middle1Object);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @param obj3_name: Object 3 wrapper
			 * @param obj4_name: Object 4 wrapper
			 * @param obj5_name: Object 5 wrapper
			 * @param obj6_name: Object 6 wrapper
			 * @param obj7_name: Object 7 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view function_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto middle1Object = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(middle1Object)) {
					middle1Object = JS_NewObject(ctx);
				}
				auto middle2Object = JS_GetPropertyStr(ctx, middle1Object, obj3_name.data());
				if (JS_IsUndefined(middle2Object)) {
					middle2Object = JS_NewObject(ctx);
				}
				auto middle3Object = JS_GetPropertyStr(ctx, middle2Object, obj4_name.data());
				if (JS_IsUndefined(middle3Object)) {
					middle3Object = JS_NewObject(ctx);
				}
				auto middle4Object = JS_GetPropertyStr(ctx, middle3Object, obj5_name.data());
				if (JS_IsUndefined(middle4Object)) {
					middle4Object = JS_NewObject(ctx);
				}
				auto middle5Object = JS_GetPropertyStr(ctx, middle4Object, obj6_name.data());
				if (JS_IsUndefined(middle5Object)) {
					middle5Object = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, middle5Object, obj7_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, middle5Object, obj7_name.data(), innerObject);
				JS_SetPropertyStr(ctx, middle4Object, obj6_name.data(), middle5Object);
				JS_SetPropertyStr(ctx, middle3Object, obj5_name.data(), middle4Object);
				JS_SetPropertyStr(ctx, middle2Object, obj4_name.data(), middle3Object);
				JS_SetPropertyStr(ctx, middle1Object, obj3_name.data(), middle2Object);
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), middle1Object);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}


			/**
			 * --------------------------------------
			 * Add C method to JS
			 * @param func: the function pointer of C
			 * @param function_name: JS Function name
			 * @param obj1_name: Object 1 wrapper
			 * @param obj2_name: Object 2 wrapper
			 * @param obj3_name: Object 3 wrapper
			 * @param obj4_name: Object 4 wrapper
			 * @param obj5_name: Object 5 wrapper
			 * @param obj6_name: Object 6 wrapper
			 * @param obj7_name: Object 7 wrapper
			 * @param obj8_name: Object 8 wrapper
			 * @return: JS function has been assigned
			 * --------------------------------------
			*/

			inline auto add_proxy(
				JSValue (*func)(JSContext *, JSValueConst, int, JSValueConst *),
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view function_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto outerObject = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(outerObject)) {
					outerObject = JS_NewObject(ctx);
				}
				auto middle1Object = JS_GetPropertyStr(ctx, outerObject, obj2_name.data());
				if (JS_IsUndefined(middle1Object)) {
					middle1Object = JS_NewObject(ctx);
				}
				auto middle2Object = JS_GetPropertyStr(ctx, middle1Object, obj3_name.data());
				if (JS_IsUndefined(middle2Object)) {
					middle2Object = JS_NewObject(ctx);
				}
				auto middle3Object = JS_GetPropertyStr(ctx, middle2Object, obj4_name.data());
				if (JS_IsUndefined(middle3Object)) {
					middle3Object = JS_NewObject(ctx);
				}
				auto middle4Object = JS_GetPropertyStr(ctx, middle3Object, obj5_name.data());
				if (JS_IsUndefined(middle4Object)) {
					middle4Object = JS_NewObject(ctx);
				}
				auto middle5Object = JS_GetPropertyStr(ctx, middle4Object, obj6_name.data());
				if (JS_IsUndefined(middle5Object)) {
					middle5Object = JS_NewObject(ctx);
				}
				auto middle6Object = JS_GetPropertyStr(ctx, middle5Object, obj7_name.data());
				if (JS_IsUndefined(middle6Object)) {
					middle6Object = JS_NewObject(ctx);
				}
				auto innerObject = JS_GetPropertyStr(ctx, middle6Object, obj8_name.data());
				if (JS_IsUndefined(innerObject)) {
					innerObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, innerObject, function_name.data(), JS_NewCFunction(ctx, func, function_name.data(), 0));
				JS_SetPropertyStr(ctx, middle6Object, obj8_name.data(), innerObject);
				JS_SetPropertyStr(ctx, middle5Object, obj7_name.data(), middle6Object);
				JS_SetPropertyStr(ctx, middle4Object, obj6_name.data(), middle5Object);
				JS_SetPropertyStr(ctx, middle3Object, obj5_name.data(), middle4Object);
				JS_SetPropertyStr(ctx, middle2Object, obj4_name.data(), middle3Object);
				JS_SetPropertyStr(ctx, middle1Object, obj3_name.data(), middle2Object);
				JS_SetPropertyStr(ctx, outerObject, obj2_name.data(), middle1Object);
				JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), outerObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value from C
			 * @param value: the C string value
			 * @param var_name: JS variable name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view var_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				JS_SetPropertyStr(ctx, global_obj, var_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value from C
			 * @param value: the integer 32 value
			 * @param var_name: JS variable name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view var_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				JS_SetPropertyStr(ctx, global_obj, var_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value from C
			 * @param value: the unsigned integer 32 value
			 * @param var_name: JS variable name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view var_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				JS_SetPropertyStr(ctx, global_obj, var_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value from C
			 * @param value: the integer 64 value
			 * @param var_name: JS variable name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view var_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				JS_SetPropertyStr(ctx, global_obj, var_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value from C
			 * @param value: the unsigned integer 64 value
			 * @param var_name: JS variable name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view var_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				JS_SetPropertyStr(ctx, global_obj, var_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewString(ctx, value.data()));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewInt32(ctx, value));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewUint32(ctx, value));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewInt64(ctx, value));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewFloat64(ctx, value));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param object_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view object_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto myObject = JS_GetPropertyStr(ctx, global_obj, object_name.data());
				if (JS_IsUndefined(myObject)) {
					myObject = JS_NewObject(ctx);
				}
				JS_SetPropertyStr(ctx, myObject, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_SetPropertyStr(ctx, global_obj, object_name.data(), myObject);
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			#pragma region comment

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			#pragma endregion

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				JS_SetPropertyStr(ctx, obj2, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				JS_SetPropertyStr(ctx, obj3, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}
			
			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				 JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				JS_SetPropertyStr(ctx, obj4, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				JS_SetPropertyStr(ctx, obj5, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				JS_SetPropertyStr(ctx, obj6, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C float value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C String value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}
			
			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}
			
			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				std::string_view value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewString(ctx, value.data()));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewInt32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			
			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				JS_SetPropertyStr(ctx, obj7, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C float value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				float value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewFloat64(ctx, static_cast<double>(value)));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C string value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				bool value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewBool(ctx, value ? 1 : 0));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C float value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint32_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewUint32(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C float value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				int64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewInt64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C float value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				uint64_t value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewBigUint64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}

			/**
			 * --------------------------------------
			 * Add a constant JS value to an object from C
			 * @param value: the C float value
			 * @param obj1_name: JS object name
			 * @param obj2_name: JS object name
			 * @param obj3_name: JS object name
			 * @param obj4_name: JS object name
			 * @param obj5_name: JS object name
			 * @param obj6_name: JS object name
			 * @param obj7_name: JS object name
			 * @param obj8_name: JS object name
			 * @param obj9_name: JS object name
			 * @param property_name: JS property name
			 * --------------------------------------
			*/

			inline auto add_constant(
				double value,
				std::string_view obj1_name,
				std::string_view obj2_name,
				std::string_view obj3_name,
				std::string_view obj4_name,
				std::string_view obj5_name,
				std::string_view obj6_name,
				std::string_view obj7_name,
				std::string_view obj8_name,
				std::string_view obj9_name,
				std::string_view property_name
			) -> void 
			{
				auto global_obj = JS_GetGlobalObject(ctx);
				auto obj1 = JS_GetPropertyStr(ctx, global_obj, obj1_name.data());
				if (JS_IsUndefined(obj1)) {
					obj1 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, global_obj, obj1_name.data(), obj1);
				}
				auto obj2 = JS_GetPropertyStr(ctx, obj1, obj2_name.data());
				if (JS_IsUndefined(obj2)) {
					obj2 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj1, obj2_name.data(), obj2);
				}
				auto obj3 = JS_GetPropertyStr(ctx, obj2, obj3_name.data());
				if (JS_IsUndefined(obj3)) {
					obj3 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj2, obj3_name.data(), obj3);
				}
				auto obj4 = JS_GetPropertyStr(ctx, obj3, obj4_name.data());
				if (JS_IsUndefined(obj4)) {
					obj4 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj3, obj4_name.data(), obj4);
				}
				auto obj5 = JS_GetPropertyStr(ctx, obj4, obj5_name.data());
				if (JS_IsUndefined(obj5)) {
					obj5 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj4, obj5_name.data(), obj5);
				}
				auto obj6 = JS_GetPropertyStr(ctx, obj5, obj6_name.data());
				if (JS_IsUndefined(obj6)) {
					obj6 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj5, obj6_name.data(), obj6);
				}
				auto obj7 = JS_GetPropertyStr(ctx, obj6, obj7_name.data());
				if (JS_IsUndefined(obj7)) {
					obj7 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj6, obj7_name.data(), obj7);
				}
				auto obj8 = JS_GetPropertyStr(ctx, obj7, obj8_name.data());
				if (JS_IsUndefined(obj8)) {
					obj8 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj7, obj8_name.data(), obj8);
				}
				auto obj9 = JS_GetPropertyStr(ctx, obj8, obj9_name.data());
				if (JS_IsUndefined(obj9)) {
					obj9 = JS_NewObject(ctx);
					JS_SetPropertyStr(ctx, obj8, obj9_name.data(), obj9);
				}
				JS_SetPropertyStr(ctx, obj8, property_name.data(), JS_NewFloat64(ctx, value));
				JS_FreeValue(ctx, global_obj);
				return;
			}



			/**
			 * Destructor
			*/

			~Runtime(

			) 
			{
				JS_FreeContext(thiz.ctx);
				JS_FreeRuntime(thiz.rt);
			}
	};
}