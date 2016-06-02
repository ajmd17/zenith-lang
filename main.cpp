#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
using std::cout;

#include "runtime/bytereader.h"
#include "runtime/vm.h"
#include "runtime/any.h"
#include "runtime/std/stdlibrary.h"

#include "compiler/parser.h"
#include "compiler/lexer.h"
#include "compiler/emit/emitter.h"

#include "util/timer.h"

#include "runtime/experimental/object.h"

using namespace zenith::compiler;
using namespace zenith::runtime;

int zen_print(const std::string &str)
{
	cout << str;
	return 0;
}

void experimentalTests()
{
	auto objMem_x = std::make_shared<Object>(3.5);

	auto objPtr = std::make_shared<Object>();
	objPtr->addMember("x", objMem_x);

	std::cout << objPtr->accessMember("x")->value<double>() << "\n";
}

void testBytecode(const std::string &str, const std::string &filename)
{
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

		zenith::runtime::StdLibrary::init();
		zenith::runtime::StdLibrary::defineAll(&emitter);

		// Run emitted code
		if (emitter.emit(emitFilename))
		{
			auto *reader = new zenith::runtime::FileByteReader(emitFilename);
			auto *vmState = new VMState(reader);
			auto *vm = new zenith::runtime::VM(vmState);

			zenith::runtime::StdLibrary::bindAll(vm);

			vm->exec();

			delete vm;
			delete vmState;
			delete reader;
		}
	}
}

int main(int argc, char *argv[])
{
	//experimentalTests();
	//system("pause");

	/*
	// For loop C++
	zenith::util::Timer timer;
	timer.start();

	for (int i = 0; i < 1000; i++)
	{
		std::cout << std::string("i * i = ") + std::to_string(i * i) << std::endl;
	}

	std::cout << "elapsed time: " << timer.elapsedTime() << "\n";
	getchar();*/
	
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
		
		testBytecode(str, filename);
	}

	system("pause");
	return 0;
}