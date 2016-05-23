#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
using std::cout;

#include "runtime/bytereader.h"
#include "runtime/vm.h"

#include "compiler/parser.h"
#include "compiler/lexer.h"
#include "compiler/emit/emitter.h"
using namespace zenith;

int zen_print(const std::string &str)
{
	cout << str << "\n";
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		cout << "Usage: " << argv[0] << " <filename> <options (not required)>\n";
	else
	{
		char *filename = argv[1];

		std::ifstream t(filename);

		if (!t.is_open())
		{
			cout << "File not found: " << filename << "\n";
			throw std::runtime_error("File not found");
		}

		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		Lexer lexer(str, filename);
		auto tokens = lexer.scan();

		Parser parser(tokens, lexer.state);
		auto unit = parser.parse();

		if (unit)
		{
			//std::cout << unit->str() << "\n\n\n";

			// Emit code
			std::string emitFilename = unit->moduleName + ".emit";
			Emitter emitter(unit.get(), parser.state);

			emitter.defineFunction({ "print", 1 });

			// Run emitted code
			if (emitter.emit(emitFilename))
			{
				auto *reader = new FileByteReader(emitFilename);
				auto *vm = new VM(reader);

				vm->bindFunction("print", zen_print);

				vm->exec();

				delete vm;
				delete reader;
			}
		}
	}

	system("pause");
	return 0;
}