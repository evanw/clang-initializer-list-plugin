#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"

using namespace clang;

namespace {
  struct CustomVisitor : RecursiveASTVisitor<CustomVisitor> {
    CompilerInstance &instance;
    unsigned int customWarning;

    CustomVisitor(CompilerInstance &instance) : instance(instance) {
      customWarning = instance.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Warning, "constructor for %0 is missing initializer for member %1");
    }

    bool VisitCXXConstructorDecl(CXXConstructorDecl *constructor) {
      // Ignore system headers
      FullSourceLoc location(constructor->getLocation(), instance.getSourceManager());
      if (location.isInSystemHeader()) {
        return true;
      }

      // Only check the constructor if it has a body
      const FunctionDecl *definitionWithBody = NULL;
      if (!constructor->hasBody(definitionWithBody) || definitionWithBody != constructor) {
        return true;
      }

      // Check each field in the enclosing record
      const CXXRecordDecl *record = constructor->getParent();
      for (CXXRecordDecl::field_iterator field = record->field_begin(), fieldEnd = record->field_end(); field != fieldEnd; field++) {

        // Ignore fields with no names (anonymous unions, for example)
        if (field->getNameAsString().empty()) {
          continue;
        }

        // Ignore fields with non-POD record types
        const CXXRecordDecl *type = field->getType()->getAsCXXRecordDecl();
        if (type && !type->isPOD()) {
          continue;
        }

        // Search for a matching initializer
        CXXConstructorDecl::init_const_iterator initializer = constructor->init_begin(), initializerEnd = constructor->init_end();
        for (; initializer != initializerEnd; initializer++) {
          if (*field == (*initializer)->getMember()) {
            break;
          }
        }

        // Emit an error for fields without a matching initializer
        if (initializer == initializerEnd) {
          instance.getDiagnostics().Report(location, customWarning) << record->getQualifiedNameAsString() << field->getNameAsString();
        }
      }

      return true;
    }
  };

  struct CustomConsumer : ASTConsumer {
    CompilerInstance &instance;

    CustomConsumer(CompilerInstance &instance) : instance(instance) {
    }

    virtual void HandleTranslationUnit(clang::ASTContext &context) {
      CustomVisitor(instance).TraverseDecl(context.getTranslationUnitDecl());
    }
  };

  struct CustomAction : PluginASTAction {
    ASTConsumer *CreateASTConsumer(CompilerInstance &instance, llvm::StringRef) {
      return new CustomConsumer(instance);
    }

    bool ParseArgs(const CompilerInstance &instance, const std::vector<std::string> &) {
      return true;
    }
  };
}

static FrontendPluginRegistry::Add<CustomAction> X("check-initializer-lists", "Check initializer lists for missing members");
