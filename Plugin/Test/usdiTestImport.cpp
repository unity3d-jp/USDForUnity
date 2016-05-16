#include <cstdio>
#include "../usdi/usdi.h"

void InspectRecursive(usdi::Schema *schema)
{
    if (!schema) { return; }

    printf("  %s (%s)\n", usdiGetPath(schema), usdiGetTypeName(schema));

    int num_children = usdiGetNumChildren(schema);
    for (int i = 0; i < num_children; ++i) {
        auto child = usdiGetChild(schema, i);
        InspectRecursive(child);
    }
}

bool TestImport(const char *path)
{
    auto *ctx = usdiOpen(path);
    if (!ctx) {
        printf("failed to load %s\n", path);
        return false;
    }
    InspectRecursive(usdiGetRoot(ctx));
    usdiDestroyContext(ctx);

    return true;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("first argument must be path to usd file\n");
        return 0;
    }
    TestImport(argv[1]);
}
