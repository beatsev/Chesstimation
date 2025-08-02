## Build, Lint, and Test

- **Build:** `pio run`
- **Upload:** `pio run --target upload`
- **Lint:** No linting is configured.
- **Test:** No testing is configured. To add tests, integrate a testing framework like `Unity` or `Catch2`.

## Code Style

- **Language:** C++ for Arduino framework.
- **Formatting:**
  - Use 4-space indentation.
  - Braces on new lines for functions and classes, same line for control structures.
- **Naming Conventions:**
  - `camelCase` for functions and variables.
  - `PascalCase` for classes.
  - `UPPER_SNAKE_CASE` for constants and macros.
- **Imports:**
  - Use `#include <...>` for libraries.
  - Use `#include "..."` for local headers.
- **Error Handling:**
  - No formal error handling. Use return values and debug prints to indicate errors.
- **Comments:**
  - Use `//` for single-line comments.
  - Use `/* ... */` for multi-line comments.
- **Types:**
  - Use `byte` for 8-bit unsigned integers.
  - Use `int` for general-purpose integers.
- **Libraries:**
  - `TFT_eSPI` for display.
  - `lvgl` for UI.
  - `BluetoothSerial` for Bluetooth communication.
- **File Naming:**
  - Use `lowercase_snake_case` for file names.
- **File Structure:**
  - `src/` contains all source code.
  - `include/` contains all header files.
  - `lib/` contains all libraries.
  - `platformio.ini` contains project configuration.

## Notes for Agents

- No `.cursor/rules/` or `.github/copilot-instructions.md` files are present.
- Ensure all changes adhere to the above guidelines.
- For new features, document build and test commands if applicable.