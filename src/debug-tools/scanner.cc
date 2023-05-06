#include <backend.hpp>
#include <scanner.hpp>

// Deletes a statement.
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

// The scanner debug tool program
int main(int argc, char** argv)
{
        printf("\n ---- DEBUG TOOL: SCANNER ---- \n\n");
        if (argc < 2) {
                rin_fatal_error(File::unknown_location(),
                        "Expected .rin file directory as command line argument");
                rin_inform(File::unknown_location(),
                        "\tSample usage: ./a.out MY_FILE.rin");
                return 0;
        }

        std::string path(argv[1]);

        Scanner sc(path);
        while (sc.has_next()) {
                Token tk = sc.next_token();
                Location l = tk.location();
                printf("%s:%d:%d: %s \t %s\n", &l.filename[0], l.line, l.column,
                       &tk.classification_as_string()[0], tk.str());
        }

        printf("\n\nSCANNER ERRORS: \n");
        sc.consume_errors();

        printf("\n ---- END DEBUG TOOL ----\n\n");

        return 0;
}
