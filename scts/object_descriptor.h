#pragma once

#include <type_traits>

#include "io.h"
#include "helpers.h"
#include "formatters.h"
#include "register_type.h"

namespace scts {
	template <auto Ptr>
	struct member {
		using value_type = typename scts::deduce_member_ptr_type<decltype(Ptr)>::type;

		static_assert(scts::is_basic_value_v<value_type> || scts::is_registered_type_v<value_type>,
			"member needs to be a basic value or a registered type!");

		template <typename O>
		static constexpr value_type& get(O& object) noexcept { return object.*Ptr; }
		template <typename O>
		static constexpr const value_type& get(const O& object) noexcept { return object.*Ptr; }
	};

	template <typename... Parents>
	struct inherits_from {
		template <typename O, typename Formatter>
		static void write(const O& object, scts::out_stream& stream) {
			write_detail::template write<O, Formatter, Parents...>(object, stream);
		}

		template <typename O, typename Formatter>
		static void read(O& object, scts::in_stream& stream) {
			read_detail::template read<O, Formatter, Parents...>(object, stream);
		}
	private:
		struct write_detail {
			template <typename O, typename Formatter>
			static void write(const O&, scts::out_stream&) { }

			template <typename O, typename Formatter, typename Parent>
			static void write(const O& object, scts::out_stream& stream) {
				write_parent<O, Formatter, Parent>(object, stream);
			}

			template <typename O, typename Formatter, typename Parent, typename Second, typename... Rest>
			static void write(const O& object, scts::out_stream& stream) {
				write_parent<O, Formatter, Parent>(object, stream);
				write<O, Second, Rest...>(object, stream);
			}

			template <typename O, typename Formatter, typename Parent>
			static void write_parent(const O& object, scts::out_stream& stream) {
				scts::register_type<Parent>::descriptor.save<Formatter>(object, stream);
				stream << ",";
			}
		};

		struct read_detail {
			template <typename O, typename Formatter>
			static void read(O&, scts::in_stream&) { }

			template <typename O, typename Formatter, typename Parent>
			static void read(O& object, scts::in_stream& stream) {
				read_parent<O, Formatter, Parent>(object, stream);
			}

			template <typename O, typename Formatter, typename Parent, typename Second, typename... Rest>
			static void read(O& object, scts::in_stream& stream) {
				read_parent<O, Formatter, Parent>(object, stream); 
				read<O, Formatter, Second, Rest...>(object, stream);
			}

			template <typename O, typename Formatter, typename Parent>
			static void read_parent(O& object, scts::in_stream& stream) {
				scts::register_type<Parent>::descriptor.load<Formatter>(object, stream);
			}
		};
	};

	template <typename... Members>
	struct members { 
		static constexpr auto member_count = sizeof...(Members);
		using name_container = std::array<std::string_view, member_count>;

		template <typename O, typename Formatter>
		static scts::out_stream& save(const O& object, scts::out_stream& stream, const name_container& names) {
			using writer = basic_writer<O, name_container, Formatter>;
			return writer::template write<Members...>(object, stream, names, 0);
		}

		template <typename O, typename Formatter>
		static O& load(O& object, scts::in_stream& stream) {
			using reader = basic_reader<O, Formatter>;
			return reader::template read<Members...>(object, stream);
		}
	};

	template <typename O, typename Members, typename InheritsFrom = inherits_from<>>
	struct object_descriptor {
		template <typename... Names>
		constexpr object_descriptor(Names... names) noexcept : m_names({ names... }) { 
			static_assert(sizeof...(Names) == Members::member_count, "object_descriptor needs the correct amount of names");
		}

		template <typename Formatter>
		scts::out_stream& save(const O& object, scts::out_stream& stream) const {
			InheritsFrom::template write<O, Formatter>(object, stream);
			return Members::template save<O, Formatter>(object, stream, m_names);
		}

		template <typename Formatter>
		O& load(O& object, scts::in_stream& stream) const {
			InheritsFrom::template read<O, Formatter>(object, stream);
			return Members::template load<O, Formatter>(object, stream);
		}
	private:
		const typename Members::name_container m_names;
	};
}