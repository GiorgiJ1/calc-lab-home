# C Arithmetic Parser

A recursive-descent arithmetic parser written in C that evaluates mathematical expressions with operator precedence, exponentiation, parentheses, and error reporting. It also supports evaluating expressions from text files.

## Features

- **Arithmetic operations:** Addition (`+`), subtraction (`-`), multiplication (`*`), and division (`/`)
- **Exponentiation:** Supports `**` with right-associative evaluation
- **Parentheses:** Groups expressions to control evaluation order
- **Operator precedence:** Follows standard mathematical rules
- **Error reporting:** Detects invalid expressions and reports error positions
- **File-based evaluation:** Reads expressions from files and saves the results
- **Recursive-descent parsing:** Implements a parser using separate functions for different grammar rules
- **Written in C:** Uses standard C features and dynamic memory management

## Requirements

- GCC or another compatible C compiler
- A C17-compatible environment
- The math library (`libm`) on systems that require explicit linking

## Build

Clone the repository:

```bash
git clone https://github.com/GiorgiJ1/calc-lab-home.git
cd calc-lab-home
```

Compile the program:

```bash
gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
```

Run it using the command-line interface supported by the program.

## Examples

The parser is designed to evaluate expressions such as:

| Expression | Result |
|---|---:|
| `2 + 3 * 4` | `14` |
| `(2 + 3) * 4` | `20` |
| `2 ** 3` | `8` |
| `2 ** 3 ** 2` | `512` |
| `10 / 2 - 1` | `4` |

Exponentiation is right-associative, so `2 ** 3 ** 2` is evaluated as `2 ** (3 ** 2)`.

## How It Works

The parser uses a **recursive-descent architecture**, where each parsing function handles a different level of operator precedence.

- `parse_expr()` handles addition and subtraction.
- `parse_term()` handles multiplication and division.
- `parse_power()` handles exponentiation.
- `parse_primary()` handles numbers and parenthesized expressions.
- `parse_number()` reads numeric values from the input.

This structure makes the parsing process easier to follow and keeps operator precedence explicit.

## Error Handling

The parser detects invalid input instead of silently accepting malformed expressions. When an error occurs, it records the position where the problem was detected.

Examples of invalid input include:

```text
2 + * 3
(4 + 5
10 / 
```

The exact error output depends on the error type and the input-processing mode.

## Project Structure

```text
calc-lab-home/
├── calc.c
└── README.md
```

## License

No license has been specified yet.
