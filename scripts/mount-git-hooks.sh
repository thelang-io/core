#!/usr/bin/env bash

set -e

cat > .git/hooks/pre-commit << EOF
#!/usr/bin/env bash

mkdir -p build
(cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug)
cmake --build build
(cd build && ctest --output-on-failure)
EOF

chmod +x .git/hooks/pre-commit
