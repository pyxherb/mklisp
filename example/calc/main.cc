#include <mklisp/runtime.h>
#include <mklisp/parser.h>
#include <fstream>

int main() {
	std::string src;
	{
		std::ifstream fs;
		try {
			fs.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
			fs.open("calc.lsp", std::ios_base::in | std::ios_base::binary);

			fs.seekg(0, std::ios::end);
			std::streampos size = fs.tellg();
			assert(size > 0);
			fs.seekg(0, std::ios::beg);

			src.resize(size);

			fs.read(src.data(), size);
		} catch (std::ios::failure e) {
			printf("Error loading main module: %s, at file offset %zu\n", e.what(), (size_t)fs.tellg());
			return -1;
		}
	}

	std::unique_ptr<mklisp::Runtime> runtime = std::make_unique<mklisp::Runtime>(std::pmr::get_default_resource());

	{
		mklisp::HostObjectRef<mklisp::NativeFnObject> printObject = mklisp::NativeFnObject::alloc(
			runtime.get(),
			[](mklisp::Context *context) {
				auto &curFrame = context->frameList.back();
				for (auto &i : curFrame.curEvalList->elements) {
					switch (i.valueType) {
						case mklisp::ValueType::Object: {
							mklisp::Object *object = i.exData.asObject;

							switch (object->getObjectType()) {
								case mklisp::ObjectType::String:
									printf("%s", ((mklisp::StringObject *)object)->data.c_str());
							}
						}
					}
				}
			});

		mklisp::Context context;
		context.bindings["print"] = printObject.get();

		mklisp::Lexer lexer;
		lexer.lex(std::pmr::get_default_resource(), src);
		mklisp::Parser parser(runtime.get());

		mklisp::HostRefHolder refHolder;
		mklisp::HostObjectRef<mklisp::ListObject> listObject;

		parser.parse(&lexer, listObject, refHolder);

		for (auto &i : listObject->elements) {
			runtime->eval(i, &context);
		}
	}

	return 0;
}
