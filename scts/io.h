#pragma once

namespace scts {
	template <typename O, typename Names, typename Formatter>
	struct basic_writer {
		template <typename Member>
		static scts::out_stream& write(const O& object, scts::out_stream& stream, const Names& names, std::size_t name_index) {
			Formatter::write_member(Member::get(object), stream, names.at(name_index), true);
			return stream;
		}

		template <typename Member, typename Second, typename... Rest>
		static scts::out_stream& write(const O& object, scts::out_stream& stream, const Names& names, std::size_t name_index) {
			Formatter::write_member(Member::get(object), stream, names.at(name_index), false);
			return write<Second, Rest...>(object, stream, names, name_index + 1);
		}
	};

	template <typename O, typename Formatter>
	struct basic_reader {
		template <typename Member>
		static O& read(O& object, scts::in_stream& stream) {
			Formatter::read_member(Member::get(object), stream);
			return object;
		}

		template <typename Member, typename Second, typename... Rest>
		static O& read(O& object, scts::in_stream& stream) {
			Formatter::read_member(Member::get(object), stream);
			return read<Second, Rest...>(object, stream);
		}
	};
}