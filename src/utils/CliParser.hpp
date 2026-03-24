#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>

/// CLI argument parser
class CliParser
{
public:
	class FlagSpec;
	class FlagArgSpec;
	class PositionalSpec;

	explicit CliParser(std::string program_name,
					   std::string description = {});
	~CliParser();

	CliParser(const CliParser &) = delete;
	CliParser &operator=(const CliParser &) = delete;
	CliParser(CliParser &&) = default;
	CliParser &operator=(CliParser &&) = default;

	// argument/flag registration

	/// Register a flag
	[[nodiscard]] FlagSpec &add_flag(std::string long_name, std::string description = {});

	/// Register a flag that takes a value: --name VALUE  or  --name=VALUE
	[[nodiscard]] FlagArgSpec &add_flag_arg(std::string long_name, std::string description = {});

	/// Register the next positional argument (order of calls = capture order)
	[[nodiscard]] PositionalSpec &add_positional(std::string meta_name, std::string description = {});

	/// Register an '-h' flag to print help text. Exits the program with the given exit code if one is provided
	void add_help_flag(std::optional<int> exit_code = std::nullopt);

	/// Parse an (argc, argv) pair from main()
	/// argv[0] (program name) is automatically skipped
	/// Throws CliError with a human-readable message on any error
	void parse(int argc, const char *const *argv);

	/// Parse an explicit list of tokens (argv[0] NOT included)
	void parse(const std::vector<std::string_view> &tokens);

	/// Returns a formatted help string
	[[nodiscard]] std::string help_text() const;

	/// Prints help to stdout
	void print_help() const;

	class FlagSpec
	{
	public:
		~FlagSpec();
		FlagSpec(const FlagSpec &) = delete;
		FlagSpec &operator=(const FlagSpec &) = delete;

		/// Assign a single-character short name (e.g. 'v' → -v)
		FlagSpec &short_name(char c);

		/// Allow this flag to be passed more than once
		FlagSpec &allow_multi();

		/// True if the flag appeared at least once
		[[nodiscard]] bool present() const;

		/// Number of times the flag appeared (always 0 or 1 unless allow_multi() set)
		[[nodiscard]] int count() const;
		[[nodiscard]] explicit operator bool() const { return present(); }

	private:
		friend class CliParser;
		struct Impl;
		explicit FlagSpec(std::unique_ptr<Impl>);
		std::unique_ptr<Impl> impl_;
	};

	class FlagArgSpec
	{
	public:
		~FlagArgSpec();
		FlagArgSpec(const FlagArgSpec &) = delete;
		FlagArgSpec &operator=(const FlagArgSpec &) = delete;

		/// Assign a single-character short name (e.g. 'o' → -o)
		FlagArgSpec &short_name(char c);

		/// Mark this argument as mandatory (parse() throws if absent)
		FlagArgSpec &required();

		/// Provide a fallback used when the flag is absent and not required
		FlagArgSpec &default_value(std::string val);

		/// Returns the value, or the default, or "" if absent and no default
		[[nodiscard]] std::string value() const;

		/// Returns nullopt when the flag was absent (and no default was set)
		[[nodiscard]] std::optional<std::string> maybe_value() const;

		[[nodiscard]] bool present() const;
		[[nodiscard]] explicit operator bool() const { return present(); }

	private:
		friend class CliParser;
		struct Impl;
		explicit FlagArgSpec(std::unique_ptr<Impl>);
		std::unique_ptr<Impl> impl_;
	};

	class PositionalSpec
	{
	public:
		~PositionalSpec();
		PositionalSpec(const PositionalSpec &) = delete;
		PositionalSpec &operator=(const PositionalSpec &) = delete;

		/// Throw during parse() if no token fills this positional slot
		PositionalSpec &required();

		/// Fallback used when the slot is absent and not required
		PositionalSpec &default_value(std::string val);

		/// Returns the captured string, the default, or "" if absent
		[[nodiscard]] std::string value() const;

		[[nodiscard]] std::optional<std::string> maybe_value() const;

		[[nodiscard]] bool present() const;
		[[nodiscard]] explicit operator bool() const { return present(); }

	private:
		friend class CliParser;
		struct Impl;
		explicit PositionalSpec(std::unique_ptr<Impl>);
		std::unique_ptr<Impl> impl_;
	};

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

/// Wrapper around runtime error for CLI-specific errors
class CliError : public std::runtime_error
{
public:
	CliError(std::string s);
};