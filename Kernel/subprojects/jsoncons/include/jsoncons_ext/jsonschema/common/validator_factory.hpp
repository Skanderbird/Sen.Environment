// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_JSONSCHEMA_COMMON_VALIDATOR_FACTORY_HPP
#define JSONCONS_EXT_JSONSCHEMA_COMMON_VALIDATOR_FACTORY_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <jsoncons/config/compiler_support.hpp>

#include <jsoncons_ext/jsonschema/common/compilation_context.hpp>
#include <jsoncons_ext/jsonschema/common/schema_validator.hpp>
#include <jsoncons_ext/jsonschema/evaluation_options.hpp>

namespace jsoncons {
namespace jsonschema {

    template <typename Json>
    using resolve_uri_type = std::function<Json(const jsoncons::uri & /*id*/)>;

    template <typename Json>
    class validator_factory
    {
    public:
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        using validator_factory_factory_type = std::function<std::unique_ptr<validator_factory<Json>>(const Json&,
            const evaluation_options&,schema_store_type*,const std::vector<resolve_uri_type<Json>>&,
            const std::unordered_map<std::string,bool>&)>;
        using keyword_validator_ptr_type = typename std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_ptr_type = typename std::unique_ptr<schema_validator<Json>>;
        using ref_validator_type = ref_validator<Json>;
        using ref_type = ref<Json>;
        using anchor_uri_map_type = std::unordered_map<std::string,uri_wrapper>;

    private:
        std::string spec_version_;
        std::unique_ptr<Json> root_schema_;
        validator_factory_factory_type factory_factory_;
        evaluation_options options_;
        schema_store_type* schema_store_ptr_;
        std::vector<resolve_uri_type<Json>> resolve_funcs_;
        std::unordered_map<std::string,bool> vocabulary_;

        schema_validator_ptr_type root_;       
        
        // Owns external schemas
        std::vector<schema_validator_ptr_type> schema_validators_;
    public:
        std::vector<std::pair<jsoncons::uri, ref_type*>> unresolved_refs_; 
        std::map<jsoncons::uri, Json> unknown_keywords_;

    public:

        validator_factory(const std::string& version, Json&& root_schema, const validator_factory_factory_type& factory_factory,
            evaluation_options options, schema_store_type* schema_store_ptr,
            const std::vector<resolve_uri_type<Json>>& resolve_funcs)
            : spec_version_(version), factory_factory_(factory_factory), options_(std::move(options)),
              schema_store_ptr_(schema_store_ptr), resolve_funcs_(resolve_funcs)
        {
            JSONCONS_ASSERT(schema_store_ptr != nullptr);
            root_schema_ = jsoncons::make_unique<Json>(std::move(root_schema));
        }

        validator_factory(const std::string& version, Json&& root_schema, const validator_factory_factory_type& factory_factory,
            evaluation_options options, schema_store_type* schema_store_ptr,
            const std::vector<resolve_uri_type<Json>>& resolve_funcs,
            const std::unordered_map<std::string,bool>& vocabulary)
            : spec_version_(version), factory_factory_(factory_factory), options_(std::move(options)),
              schema_store_ptr_(schema_store_ptr), resolve_funcs_(resolve_funcs), vocabulary_(vocabulary)
        {
            JSONCONS_ASSERT(schema_store_ptr != nullptr);
            root_schema_ = jsoncons::make_unique<Json>(std::move(root_schema));
        }

        virtual ~validator_factory() = default;

        const std::unordered_map<std::string,bool>& vocabulary() const {return vocabulary_;}
        
        void save_schema(schema_validator_ptr_type&& schema)
        {
            schema_validators_.emplace_back(std::move(schema));
        }

        const std::string& schema() const
        {
            return spec_version_;
        }
        
        void build_schema() 
        {
            anchor_uri_map_type anchor_dict;
            root_ = make_schema_validator(compilation_context(uri_wrapper(options_.default_base_uri())), *root_schema_, {}, anchor_dict);
        }

        void build_schema(const std::string& retrieval_uri) 
        {
            anchor_uri_map_type anchor_dict;
            root_ = make_schema_validator(compilation_context(uri_wrapper(retrieval_uri)), *root_schema_, {}, anchor_dict);
        }

        evaluation_options options() const
        {
            return options_;
        }
        
        schema_validator_ptr_type make_boolean_schema(const compilation_context& context, const Json& sch)
        {
            uri schema_location = context.get_base_uri();
            schema_validator_ptr_type schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>( 
                schema_location, sch.template as<bool>());

            return schema_validator_ptr;
        }
        
        std::unique_ptr<document_schema_validator<Json>> get_schema_validator()
        {                        
            //std::cout << "schema_store:\n";
            //for (auto& member : *schema_store_ptr_)
            //{
            //    std::cout << "    " << member.first.string() << "\n";
            //}

            // load all external schemas that have not already been loaded           
            // new unresolved refs may be added to the end as earlier ones are resolved
            for (std::size_t i = 0; i < unresolved_refs_.size(); ++i)
            {
                auto loc = unresolved_refs_[i].first;
                //std::cout << "unresolved: " << loc.string() << "\n";
                if (schema_store_ptr_->find(loc) == schema_store_ptr_->end()) // registry for this file is empty
                {
                    bool found = false;
                    for (auto it = resolve_funcs_.begin(); it != resolve_funcs_.end() && !found; ++it)
                    {
                        Json external_sch = (*it)(loc);

                        if (external_sch.is_object() || external_sch.is_bool())
                        {
                            anchor_uri_map_type anchor_dict2;
                            this->save_schema(make_cross_draft_schema_validator(compilation_context(uri_wrapper(loc.base())), 
                                std::move(external_sch), {}, anchor_dict2));
                            found = true;
                        }
                    }
                    if (found)
                    {
                        // Try resolving again
                        if (schema_store_ptr_->find(loc) == schema_store_ptr_->end()) 
                        {
                            JSONCONS_THROW(jsonschema::schema_error("Unresolved reference '" + loc.string() + "'"));
                        }
                    }
                    else
                    {
                        JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema '" + loc.base().string() + "'" ));
                    }
                }
            }

            resolve_references();

            return jsoncons::make_unique<document_schema_validator<Json>>(std::move(root_schema_), std::move(root_), std::move(schema_validators_));
        }

        void resolve_references()
        {
            for (auto& ref : unresolved_refs_)
            {
                auto it = schema_store_ptr_->find(ref.first);
                if (it == schema_store_ptr_->end())
                {
                    JSONCONS_THROW(schema_error("Undefined reference " + ref.first.string()));
                }
                if ((*it).second == nullptr)
                {
                    JSONCONS_THROW(schema_error("Null referred schema " + ref.first.string()));
                }
                ref.second->set_referred_schema((*it).second);
            }
        }

        void insert_schema(const uri_wrapper& identifier, schema_validator<Json>* s)
        {
            this->schema_store_ptr_->emplace(identifier.uri(), s);
        }

        void insert_unknown_keyword(const uri_wrapper& uri, 
                                    const std::string& key, 
                                    const Json& value)
        {
            auto new_u = uri.append(key);
            uri_wrapper new_uri(new_u);

            if (new_uri.has_fragment() && !new_uri.has_plain_name_fragment()) 
            {
                // is there a reference looking for this unknown-keyword, which is thus no longer a unknown keyword but a schema
                auto unresolved_refs = std::find_if(this->unresolved_refs_.begin(), this->unresolved_refs_.end(),
                    [new_uri](const std::pair<jsoncons::uri,ref<Json>*>& pr) {return pr.first == new_uri.uri();});
                if (unresolved_refs != this->unresolved_refs_.end())
                {
                    anchor_uri_map_type anchor_dict2;
                    this->save_schema(make_cross_draft_schema_validator(compilation_context(new_uri), value, {}, anchor_dict2));
                }
                else // no, nothing ref'd it, keep for later
                {
                    //file.unknown_keywords.emplace(fragment, value);
                    this->unknown_keywords_.emplace(new_uri.uri(), value);
                }

                // recursively add possible subschemas of unknown keywords
                if (value.type() == json_type::object_value)
                {
                    for (const auto& subsch : value.object_range())
                    {
                        insert_unknown_keyword(new_uri, subsch.key(), subsch.value());
                    }
                }
            }
        }

        std::unique_ptr<ref_validator<Json>> get_or_create_reference(const Json& schema, const uri_wrapper& identifier)
        {
            // a schema already exists
            auto it = this->schema_store_ptr_->find(identifier.uri());
            if (it != this->schema_store_ptr_->end())
            {
                return jsoncons::make_unique<ref_validator_type>(schema, identifier.uri(), (*it).second);
            }

            // referencing an unknown keyword, turn it into schema
            //
            // an unknown keyword can only be referenced by a JSONPointer,
            // not by a plain name identifier
            if (identifier.has_fragment() && !identifier.has_plain_name_fragment()) 
            {
                //std::string fragment = std::string(identifier.fragment());

                auto it2 = this->unknown_keywords_.find(identifier.uri());
                if (it2 != this->unknown_keywords_.end())
                {
                    auto& subsch = it2->second;
                    anchor_uri_map_type anchor_dict2;
                    auto s = make_cross_draft_schema_validator(compilation_context(identifier), subsch, {}, anchor_dict2);
                    this->unknown_keywords_.erase(it2);
                    auto orig = jsoncons::make_unique<ref_validator_type>(schema, identifier.uri(), s.get());
                    this->save_schema(std::move(s));
                    return orig;
                }
            }

            // get or create a ref_validator
            auto orig = jsoncons::make_unique<ref_validator_type>(schema, identifier.uri());

            this->unresolved_refs_.emplace_back(identifier.uri(), orig.get());
            return orig;
        }

        static bool validate_anchor(const std::string& s)
        {
            if (s.empty())
            {
                return false;
            }
            if (!((s[0] >= 'a' && s[0] <= 'z') || (s[0] >= 'A' && s[0] <= 'Z')))
            {
                return false;
            }

            for (std::size_t i = 1; i < s.size(); ++i)
            {
                switch (s[i])
                {
                    case '-':
                    case '_':
                    case ':':
                    case '.':
                        break;
                    default:
                        if (!((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= '0' && s[i] <= '9')))
                        {
                            return false;
                        }
                        break;
                }
            }
            return true;
        }

        virtual compilation_context make_compilation_context(const compilation_context& parent,
            const Json& sch, jsoncons::span<const std::string> keys) const = 0;

        virtual schema_validator_ptr_type make_schema_validator(const compilation_context& context, 
            const Json& sch, jsoncons::span<const std::string> keys, anchor_uri_map_type& anchor_dict) = 0;

        schema_validator_ptr_type make_cross_draft_schema_validator(const compilation_context& context, 
            const Json& sch, jsoncons::span<const std::string> keys, anchor_uri_map_type& anchor_dict)
        {
            schema_validator_ptr_type schema_val = schema_validator_ptr_type{};
            switch (std::move(sch).type())
            {
                case json_type::object_value:
                {
                    auto it = std::move(sch).find("$schema");
                    if (it != std::move(sch).object_range().end())
                    {
                        if ((*it).value().as_string_view() == schema())
                        {
                            return make_schema_validator(context, std::move(sch), keys, anchor_dict);
                        }
                        else
                        {
                            auto validator_factory = factory_factory_(std::move(sch), options_, schema_store_ptr_, resolve_funcs_, vocabulary_);
                            validator_factory->build_schema(context.get_base_uri().string());
                            schema_val = validator_factory->get_schema_validator();
                        }
                    }
                    else
                    {
                        return make_schema_validator(context, std::move(sch), keys, anchor_dict);
                    }
                    break;
                }
                case json_type::bool_value:
                {
                    return make_schema_validator(context, std::move(sch), keys, anchor_dict);
                }
                default:
                    JSONCONS_THROW(schema_error("Schema must be object or boolean"));
            }
            return schema_val;
        }

        std::unique_ptr<properties_validator<Json>> make_properties_validator(const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            std::string keyword = "properties";
            uri schema_location = context.get_base_uri();

            std::map<std::string, schema_validator_ptr_type> properties;

            for (const auto& prop : sch.object_range())
            {
                std::string sub_keys[] = {keyword, prop.key()};
                properties.emplace(std::make_pair(prop.key(), 
                    this->make_cross_draft_schema_validator(context, prop.value(), sub_keys, anchor_dict)));
            }

            return jsoncons::make_unique<properties_validator<Json>>(parent, std::move(schema_location),
                context.get_custom_message(keyword), std::move(properties));
        }

        virtual std::unique_ptr<max_length_validator<Json>> make_max_length_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("maxLength");
            if (!sch.is_number())
            {
                const std::string message("maxLength must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_length_validator<Json>>(parent, schema_location, context.get_custom_message("maxLength"), 
                value);
        }

        virtual std::unique_ptr<min_length_validator<Json>> make_min_length_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("minLength");
            if (!sch.is_number())
            {
                const std::string message("minLength must be an integer value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_length_validator<Json>>(parent, schema_location, context.get_custom_message("minLength"), value);
        }

        virtual std::unique_ptr<not_validator<Json>> make_not_validator(const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.make_schema_location("not");
            std::string not_key[] = { "not" };
            return jsoncons::make_unique<not_validator<Json>>(parent, schema_location, context.get_custom_message("not"), 
                make_cross_draft_schema_validator(context, sch, not_key, anchor_dict));
        }

        virtual std::unique_ptr<const_validator<Json>> make_const_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("const");
            return jsoncons::make_unique<const_validator<Json>>(parent, schema_location, 
                context.get_custom_message("const"), sch);
        }

        virtual std::unique_ptr<enum_validator<Json>> make_enum_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("enum");
            return jsoncons::make_unique<enum_validator<Json>>(parent, schema_location, context.get_custom_message("enum"), sch);
        }

        virtual std::unique_ptr<required_validator<Json>> make_required_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("required");
            return jsoncons::make_unique<required_validator<Json>>(parent, schema_location, 
                context.get_custom_message("required"), sch.template as<std::vector<std::string>>());
        }

        virtual std::unique_ptr<maximum_validator<Json>> make_maximum_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("maximum");
            if (!sch.is_number())
            {
                const std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            return jsoncons::make_unique<maximum_validator<Json>>(parent, schema_location, 
                context.get_custom_message("maximum"), sch);
        }

        virtual std::unique_ptr<exclusive_maximum_validator<Json>> make_exclusive_maximum_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("exclusiveMaximum");
            if (!sch.is_number())
            {
                const std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            return jsoncons::make_unique<exclusive_maximum_validator<Json>>(parent, schema_location, 
                context.get_custom_message("exclusiveMaximum"), sch);
        }

        virtual std::unique_ptr<keyword_validator<Json>> make_minimum_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("minimum");
                
            if (!sch.is_number())
            {
                const std::string message("minimum must be an integer");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            return jsoncons::make_unique<minimum_validator<Json>>(parent, schema_location, 
                context.get_custom_message("minimum"), sch);
        }

        virtual std::unique_ptr<exclusive_minimum_validator<Json>> make_exclusive_minimum_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("exclusiveMinimum");
            if (!sch.is_number())
            {
                const std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            return jsoncons::make_unique<exclusive_minimum_validator<Json>>(parent, schema_location, 
                context.get_custom_message("exclusiveMinimum"), sch);
        }

        virtual std::unique_ptr<multiple_of_validator<Json>> make_multiple_of_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("multipleOf");
            if (!sch.is_number())
            {
                const std::string message("multipleOf must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<multiple_of_validator<Json>>(parent, schema_location, 
                context.get_custom_message("multipleOf"), value);
        }


        virtual std::unique_ptr<type_validator<Json>> make_type_validator(const compilation_context& context,
            const Json& sch, const Json& parent)
        {
            std::string keyword = "type";
            uri schema_location = context.get_base_uri();
            
            std::vector<json_schema_type> expected_types;

            switch (sch.type()) 
            { 
                case json_type::string_value: 
                {
                    auto type = sch.template as<std::string>();
                    if (type == "null")
                    {
                        expected_types.push_back(json_schema_type::null);
                    }
                    else if (type == "object")
                    {
                        expected_types.push_back(json_schema_type::object);
                    }
                    else if (type == "array")
                    {
                        expected_types.push_back(json_schema_type::array);
                    }
                    else if (type == "string")
                    {
                        expected_types.push_back(json_schema_type::string);
                    }
                    else if (type == "boolean")
                    {
                        expected_types.push_back(json_schema_type::boolean);
                    }
                    else if (type == "integer")
                    {
                        expected_types.push_back(json_schema_type::integer);
                    }
                    else if (type == "number")
                    {
                        expected_types.push_back(json_schema_type::number);
                    }
                    break;
                } 

                case json_type::array_value: // "type": ["type1", "type2"]
                {
                    for (const auto& item : sch.array_range())
                    {
                        auto type = item.template as<std::string>();
                        if (type == "null")
                        {
                            expected_types.push_back(json_schema_type::null);
                        }
                        else if (type == "object")
                        {
                            expected_types.push_back(json_schema_type::object);
                        }
                        else if (type == "array")
                        {
                            expected_types.push_back(json_schema_type::array);
                        }
                        else if (type == "string")
                        {
                            expected_types.push_back(json_schema_type::string);
                        }
                        else if (type == "boolean")
                        {
                            expected_types.push_back(json_schema_type::boolean);
                        }
                        else if (type == "integer")
                        {
                            expected_types.push_back(json_schema_type::integer);
                        }
                        else if (type == "number")
                        {
                            expected_types.push_back(json_schema_type::number);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            
            return jsoncons::make_unique<type_validator<Json>>(parent, std::move(schema_location), context.get_custom_message(keyword), 
                std::move(expected_types));
        }

        virtual std::unique_ptr<content_encoding_validator<Json>> make_content_encoding_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            std::string keyword = "contentEncoding";
            uri schema_location = context.make_schema_location(keyword);
            if (!sch.is_string())
            {
                const std::string message("contentEncoding must be a string");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::string>();
            return jsoncons::make_unique<content_encoding_validator<Json>>(parent, schema_location, 
                context.get_custom_message(keyword), value);
        }

        virtual std::unique_ptr<content_media_type_validator<Json>> make_content_media_type_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            std::string keyword = "contentMediaType";
            uri schema_location = context.make_schema_location(keyword);
            if (!sch.is_string())
            {
                const std::string message("contentMediaType must be a string");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            
            std::string content_encoding;
            auto it = parent.find("contentEncoding");
            if (it != parent.object_range().end())
            {
                if (!(*it).value().is_string())
                {
                    const std::string message("contentEncoding must be a string");
                    JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
                }

                content_encoding = (*it).value().as_string();
            }
            
            auto value = sch.template as<std::string>();
            return jsoncons::make_unique<content_media_type_validator<Json>>(parent, schema_location, 
                context.get_custom_message(keyword), value, content_encoding);
        }

        virtual std::unique_ptr<format_validator<Json>> make_format_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            auto schema_location = context.make_schema_location("format");

            std::string format = sch.template as<std::string>();

            std::string message_key;
            validate_format validate;
            if (format == "date-time")
            {
                message_key = "format.date-time";
                validate = rfc3339_date_time_check;
            }
            else if (format == "date") 
            {
                message_key = "format.date";
                validate = rfc3339_date_check;
            } 
            else if (format == "time") 
            {
                message_key = "format.time";
                validate = rfc3339_time_check;
            } 
            else if (format == "email") 
            {
                message_key = "format.email";
                validate = email_check;
            } 
            else if (format == "hostname") 
            {
                message_key = "format.hostname";
                validate = hostname_check;
            } 
            else if (format == "ipv4") 
            {
                message_key = "format.ipv4";
                validate = ipv4_check;
            } 
            else if (format == "ipv6") 
            {
                message_key = "format.ipv6";
                validate = ipv6_check;
            } 
            else if (format == "regex") 
            {
                message_key = "format.regex";
                validate = regex_check;
            } 
            else if (format == "json-pointer") 
            {
                message_key = "format.json-pointer";
                validate = jsonpointer_check;
            } 
            else if (format == "uri") 
            {
                message_key = "format.uri";
                validate = uri_check;
            } 
            else if (format == "uri-reference") 
            {
                message_key = "format.uri-reference";
                validate = uri_reference_check;
            } 
            else
            {
                // Not supported - ignore
                validate = nullptr;
            }       

            return jsoncons::make_unique<format_validator<Json>>(parent, schema_location, context.get_custom_message(message_key), 
                validate);
        }

        virtual std::unique_ptr<pattern_validator<Json>> make_pattern_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("pattern");
            auto pattern_string = sch.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>(parent, schema_location, context.get_custom_message("pattern"), 
                pattern_string, regex);
        }

        virtual std::unique_ptr<max_items_validator<Json>> make_max_items_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            std::string keyword = "maxItems";
            uri schema_location = context.make_schema_location(keyword);

            if (!sch.is_number())
            {
                const std::string message("maxItems must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_items_validator<Json>>(parent, schema_location, context.get_custom_message(keyword), value);
        }

        virtual std::unique_ptr<min_items_validator<Json>> make_min_items_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            std::string keyword = "minItems";
            uri schema_location = context.make_schema_location(keyword);

            if (!sch.is_number())
            {
                const std::string message("minItems must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_items_validator<Json>>(parent, schema_location, context.get_custom_message(keyword), value);
        }

        virtual std::unique_ptr<max_properties_validator<Json>> make_max_properties_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            std::string keyword = "maxProperties";
            uri schema_location = context.make_schema_location(keyword);
            if (!sch.is_number())
            {
                const std::string message("maxProperties must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_properties_validator<Json>>(parent, schema_location, context.get_custom_message(keyword), value);
        }

        virtual std::unique_ptr<min_properties_validator<Json>> make_min_properties_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            std::string keyword = "minProperties";
            uri schema_location = context.make_schema_location(keyword);
            if (!sch.is_number())
            {
                const std::string message("minProperties must be a number value");
                JSONCONS_THROW(schema_error(schema_location.string() + ": " + message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_properties_validator<Json>>(parent, schema_location, context.get_custom_message(keyword), value);
        }

        virtual std::unique_ptr<contains_validator<Json>> make_contains_validator(const compilation_context& context,
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.make_schema_location("contains");

            std::string sub_keys[] = { "contains" };

            std::unique_ptr<max_contains_keyword<Json>> max_contains;
            auto it = parent.find("maxContains");
            if (it != parent.object_range().end()) 
            {
                uri path = context.make_schema_location("maxContains");
                auto value = (*it).value().template as<std::size_t>();
                max_contains = jsoncons::make_unique<max_contains_keyword<Json>>(parent, path, context.get_custom_message("maxContains"), value);
            }
            else
            {
                uri path = context.make_schema_location("maxContains");
                max_contains = jsoncons::make_unique<max_contains_keyword<Json>>(parent, path, context.get_custom_message("maxContains"), (std::numeric_limits<std::size_t>::max)());
            }

            std::unique_ptr<min_contains_keyword<Json>> min_contains;
            it = parent.find("minContains");
            if (it != parent.object_range().end()) 
            {
                uri path = context.make_schema_location("minContains");
                auto value = (*it).value().template as<std::size_t>();
                min_contains = jsoncons::make_unique<min_contains_keyword<Json>>(parent, path, context.get_custom_message("minContains"), value);
            }
            else
            {
                uri path = context.make_schema_location("minContains");
                min_contains = jsoncons::make_unique<min_contains_keyword<Json>>(parent, path, context.get_custom_message("minContains"), 1);
            }

            return jsoncons::make_unique<contains_validator<Json>>(parent, schema_location, context.get_custom_message("contains"), 
                make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict), std::move(max_contains), std::move(min_contains));
        }

        virtual std::unique_ptr<unique_items_validator<Json>> make_unique_items_validator(const compilation_context& context, 
            const Json& sch, const Json& parent)
        {
            uri schema_location = context.make_schema_location("uniqueItems");
            bool are_unique = sch.template as<bool>();
            return jsoncons::make_unique<unique_items_validator<Json>>(parent, schema_location, 
                context.get_custom_message("uniqueItems"), 
                are_unique);
        }

        virtual std::unique_ptr<all_of_validator<Json>> make_all_of_validator(const compilation_context& context,
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.make_schema_location("allOf");
            std::vector<schema_validator_ptr_type> subschemas;

            std::size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { "allOf", std::to_string(c++) };
                subschemas.emplace_back(make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
            }
            return jsoncons::make_unique<all_of_validator<Json>>(parent, std::move(schema_location), 
                context.get_custom_message("allOf"), std::move(subschemas));
        }

        virtual std::unique_ptr<any_of_validator<Json>> make_any_of_validator(const compilation_context& context,
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.make_schema_location("anyOf");
            std::vector<schema_validator_ptr_type> subschemas;

            std::size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { "anyOf", std::to_string(c++) };
                subschemas.emplace_back(make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
            }
            return jsoncons::make_unique<any_of_validator<Json>>(parent, std::move(schema_location), 
                context.get_custom_message("anyOf"), std::move(subschemas));
        }

        virtual std::unique_ptr<one_of_validator<Json>> make_one_of_validator(const compilation_context& context,
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location{ context.make_schema_location("oneOf") };
            std::vector<schema_validator_ptr_type> subschemas;

            std::size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { "oneOf", std::to_string(c++) };
                subschemas.emplace_back(make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
            }
            return jsoncons::make_unique<one_of_validator<Json>>(parent, std::move(schema_location), 
                context.get_custom_message("oneOf"), std::move(subschemas));
        }

        virtual std::unique_ptr<dependencies_validator<Json>> make_dependencies_validator(const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.get_base_uri();
            std::map<std::string, keyword_validator_ptr_type> dependent_required;
            std::map<std::string, schema_validator_ptr_type> dependent_schemas;

            //std::cout << "dependencies" << "\n" << pretty_print(sch) << "\n";

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::array_value:
                    {
                        auto location = context.make_schema_location("dependencies");
                        dependent_required.emplace(dep.key(), 
                            this->make_required_validator(compilation_context(std::vector<uri_wrapper>{{uri_wrapper{ location }}}),
                                dep.value(), sch));
                        break;
                    }
                    case json_type::bool_value:
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependencies"};
                        dependent_schemas.emplace(dep.key(),
                            make_cross_draft_schema_validator(context, dep.value(), sub_keys, anchor_dict));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }         

            return jsoncons::make_unique<dependencies_validator<Json>>(parent, std::move(schema_location),
                std::move(dependent_required), std::move(dependent_schemas));
        }

        virtual std::unique_ptr<property_names_validator<Json>> make_property_names_validator(const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.get_base_uri();
            schema_validator_ptr_type property_names_schema_validator;

            std::string sub_keys[] = { "propertyNames"};
            property_names_schema_validator = make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict);

            return jsoncons::make_unique<property_names_validator<Json>>(parent, std::move(schema_location), context.get_custom_message("propertyNames"),
                std::move(property_names_schema_validator));
        }

        // 201909 and later
                
        virtual std::unique_ptr<dependent_required_validator<Json>> make_dependent_required_validator( 
            const compilation_context& context, const Json& sch, const Json& parent)
        {
            uri schema_location = context.get_base_uri();
            std::map<std::string, keyword_validator_ptr_type> dependent_required;

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::array_value:
                    {
                        auto location = context.make_schema_location("dependentRequired");
                        dependent_required.emplace(dep.key(), 
                            this->make_required_validator(compilation_context(std::vector<uri_wrapper>{{uri_wrapper{ location }}}),
                                dep.value(), sch));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }

            return jsoncons::make_unique<dependent_required_validator<Json>>(parent, std::move(schema_location),
                std::move(dependent_required));
        }

        virtual std::unique_ptr<dependent_schemas_validator<Json>> make_dependent_schemas_validator( const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.get_base_uri();
            std::map<std::string, schema_validator_ptr_type> dependent_schemas;

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::bool_value:
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependentSchemas"};
                        dependent_schemas.emplace(dep.key(),
                            make_cross_draft_schema_validator(context, dep.value(), sub_keys, anchor_dict));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            

            return jsoncons::make_unique<dependent_schemas_validator<Json>>(parent, std::move(schema_location), context.get_custom_message("dependentSchemas"),
                std::move(dependent_schemas));
        }

        std::unique_ptr<prefix_items_validator<Json>> make_prefix_items_validator_07(const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            std::vector<schema_validator_ptr_type> prefix_item_validators;
            std::unique_ptr<items_keyword<Json>> items_val;

            uri schema_location{context.make_schema_location("items")};

            if (sch.type() == json_type::array_value) 
            {
                std::size_t c = 0;
                for (const auto& subsch : sch.array_range())
                {
                    std::string sub_keys[] = {"items", std::to_string(c++)};

                    prefix_item_validators.emplace_back(this->make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
                }

                auto it = parent.find("additionalItems");
                if (it != parent.object_range().end()) 
                {
                    uri items_location{context.make_schema_location("additionalItems")};
                    std::string sub_keys[] = {"additionalItems"};
                    items_val = jsoncons::make_unique<items_keyword<Json>>("additionalItems", parent, items_location, 
                        context.get_custom_message("additionalItems"),
                        this->make_cross_draft_schema_validator(context, (*it).value(), sub_keys, anchor_dict));
                }
            }

            return jsoncons::make_unique<prefix_items_validator<Json>>("items", parent, schema_location, context.get_custom_message("items"),
                std::move(prefix_item_validators), std::move(items_val));
        }
        
        std::unique_ptr<items_validator<Json>> make_items_validator(const std::string& keyword_name,
            const compilation_context& context, 
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location{context.make_schema_location(keyword_name)};

            std::string sub_keys[] = {keyword_name};

            return jsoncons::make_unique<items_validator<Json>>(keyword_name, parent, schema_location,
                context.get_custom_message(keyword_name), 
                this->make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict));
        }

        virtual std::unique_ptr<unevaluated_properties_validator<Json>> make_unevaluated_properties_validator(
            const compilation_context& context, const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            std::string keyword = "unevaluatedProperties";
            uri schema_location = context.get_base_uri();

            std::string sub_keys[] = {keyword};

            return jsoncons::make_unique<unevaluated_properties_validator<Json>>(parent, std::move(schema_location),
                context.get_custom_message(keyword),
                make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict));
        }

        virtual std::unique_ptr<unevaluated_items_validator<Json>> make_unevaluated_items_validator(
            const compilation_context& context, const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_location = context.get_base_uri();

            std::string sub_keys[] = {"unevaluatedItems"};

            return jsoncons::make_unique<unevaluated_items_validator<Json>>(parent, std::move(schema_location), context.get_custom_message("unevaluatedItems"),               
                make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict));
        }

        std::unique_ptr<additional_properties_validator<Json>> make_additional_properties_validator( 
            const compilation_context& context, const Json& sch, const Json& parent,
            std::unique_ptr<properties_validator<Json>>&& properties, std::unique_ptr<pattern_properties_validator<Json>>&& pattern_properties,
            anchor_uri_map_type& anchor_dict)
        {
            std::string keyword = "additionalProperties";
            uri schema_location = context.get_base_uri();

            std::vector<keyword_validator_ptr_type> validators;
            schema_validator_ptr_type additional_properties;

            std::string sub_keys[] = {keyword};
            additional_properties = this->make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict);

            return jsoncons::make_unique<additional_properties_validator<Json>>(parent, additional_properties->schema_location(),
                context.get_custom_message(keyword),
                std::move(properties), std::move(pattern_properties),
                std::move(additional_properties));
        }

    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_EXT_JSONSCHEMA_COMMON_SCHEMA_HPP
