#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"

using namespace clang;

namespace {
  struct CustomStmtVisitor : StmtVisitor<CustomStmtVisitor> {
    std::set<ValueDecl *> fields;

    void VisitStmt(Stmt *statement) {
      for (Stmt::child_range child = statement->children(); child; child++) {
        if (*child) {
          Visit(*child);
        }
      }
    }

    void VisitBinAssign(BinaryOperator *expr) {
      // Record the variables for assignments of the form "this->x = ...;"
      if (MemberExpr *member = dyn_cast<MemberExpr>(expr->getLHS())) {
        if (isa<CXXThisExpr>(member->getBase())) {
          fields.insert(member->getMemberDecl());
        }
      }

      // There may also be assignments inside the child expressions, especially those of the form "this->x = this->y = ...;"
      Visit(expr->getLHS());
      Visit(expr->getRHS());
    }
  };

  struct CustomDeclVisitor : RecursiveASTVisitor<CustomDeclVisitor> {
    CompilerInstance &instance;
    unsigned int customWarning;

    CustomDeclVisitor(CompilerInstance &instance) : instance(instance) {
      customWarning = instance.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Warning,
        "constructor for %0 is missing %plural{1:an initializer for member|:initializers for members}1 %2");
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

      // Ignore delegating constructors since it's assumed the other constructor works
      if (constructor->isDelegatingConstructor()) {
        return true;
      }

      // Scan the body for assignments
      CustomStmtVisitor visitor;
      visitor.Visit(constructor->getBody());

      // Check each field in the enclosing record
      std::vector<std::string> missing;
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

        // Ignore fields that are assigned to in the constructor body
        if (visitor.fields.count(*field)) {
          continue;
        }

        // Search for a matching initializer
        CXXConstructorDecl::init_const_iterator initializer = constructor->init_begin(), initializerEnd = constructor->init_end();
        for (; initializer != initializerEnd; initializer++) {
          if (*field == (*initializer)->getMember()) {
            break;
          }
        }

        // Record all uninitialized fields
        if (initializer == initializerEnd) {
          missing.push_back(field->getNameAsString());
        }
      }

      // Emit a single warning containing all uninitialized fields
      if (!missing.empty()) {
        std::string text;
        int n = missing.size();
        for (int i = 0; i < n; i++) {
          if (i && n == 2) text += " and ";
          else if (i && n > 2) text += i + 1 == n ? ", and " : ", ";
          text += missing[i];
        }
        instance.getDiagnostics().Report(location, customWarning) << record->getQualifiedNameAsString() << n << text;
      }

      return true;
    }
  };

  struct CustomConsumer : ASTConsumer {
    CompilerInstance &instance;

    CustomConsumer(CompilerInstance &instance) : instance(instance) {
    }

    virtual void HandleTranslationUnit(clang::ASTContext &context) {
      CustomDeclVisitor(instance).TraverseDecl(context.getTranslationUnitDecl());
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
