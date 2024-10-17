#include <lexer.hpp>
#include <compiler.hpp>
#include <fs.hpp>
#include <iostream>
#include <args.hpp>

int main(int argc, char **argv)
{
	Settings settings;
	ArgParser arg_parser = ArgParser(argc, argv);

	settings.compilation_level = CL_EXE;
	settings.filename = "";

	while (true)
	{
		std::string arg = arg_parser.next();
		if (arg == "")
			break;

		if (arg.at(0) != '-')
		{
			settings.filename = arg;
		}
		else
		{
			if (arg == "--help")
			{
				std::cout << "Usage: bfcc <file> [options]\n";
				std::cout << "Options:\n";
				std::cout << "  --help                   Display this information\n";
				std::cout << "  --ir (-i)                Generate only IR code\n";
				std::cout << "  --asm (-S)               Generate only assembly\n";
				std::cout << "  --obj (-c)               Generate only object file\n";
				return 0;
			}
			else if (arg == "--ir" || arg == "-i")
			{
				settings.compilation_level = CL_IR;
			}
			else if (arg == "--asm" || arg == "-S")
			{
				settings.compilation_level = CL_ASM;
			}
			else if (arg == "--obj" || arg == "-c")
			{
				settings.compilation_level = CL_OBJ;
			}
		}
	}

	if (settings.filename.empty())
	{
		std::cout << "\x1b[1mbfcc:\x1b[0m \x1b[1;31mfatal error:\x1b[0m no input files\n";
		std::cout << "compilation terminated.\n";
		return 1;
	}

	std::string code = readFile(settings.filename);
	Lexer lexer = Lexer(code);
	compile(lexer, settings);
}
