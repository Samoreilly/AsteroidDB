# Requirements for AsteroidDB

## Overview

AsteroidDB is a non-relational database engine that supports a subset of SQL. It aims to bridge the gap between simple SQL query interfaces and lightweight, non-relational storage backends.

## 1. Architecture Components

The system is composed of the following pipeline stages:

1.  **REPL (Read-Eval-Print Loop)**
    - Accepts user input (SQL string) from the console.
    - Displays query results or error messages.

2.  **Tokenizer (Lexer)**
    - **Input**: Raw SQL string (e.g., `SELECT name FROM database`).
    - **Action**: Scans the string and converts it into a sequence of tokens (`SELECT`, `IDENTIFIER:name`, `FROM`, `IDENTIFIER:database`).
    - **Output**: `Vector<Token>`.

3.  **Parser**
    - **Input**: `Vector<Token>`.
    - **Action**: Validates the token sequence against a simplified SQL grammar.
    - **Output**: **Abstract Syntax Tree (AST)**.
      - Example: `SelectNode` containing `columns=[name]` and `source=database`.

4.  **Query Execution Engine (Interpreter)**
    - **Input**: `AST Root Node`.
    - **Action**: Traverses the AST and invokes the corresponding storage operations.
    - **Output**: Result set (rows of data).

5.  **Storage Engine**
    - **Input**: Read/Write requests (e.g., `scan_table("database")`).
    - **Action**: Handles file I/O.
    - **Data Format**: Newline-delimited JSON (NDJSON) or binary equivalent for simplicity.
    - **Output**: Raw data records.

---

## 2. Detailed Data Flow

### Example Scenario: `SELECT name, age FROM users`

#### Step 1: Input

- User types: `SELECT name, age FROM users;`

#### Step 2: Tokenization

The raw string is broken into classified chunks:

- `[KEYWORD: SELECT]`
- `[IDENTIFIER: name]`
- `[COMMA]`
- `[IDENTIFIER: age]`
- `[KEYWORD: FROM]`
- `[IDENTIFIER: users]`
- `[SEMICOLON]`

#### Step 3: Parsing (AST Creation)

The parser identifies the statement type (`SELECT`) and builds a tree:

```
Info: SelectStatement
├── Columns
│   ├── name
│   └── age
└── Table
    └── users
```

#### Step 4: Plan / Execution

The engine sees a `SelectStatement`:

1.  Calls `Storage::get_table("users")`.
2.  Iterates through every record (Full Table Scan).
3.  For each record, filters out only `name` and `age` fields.
4.  Accumulates results into a `ResultSet`.

#### Step 5: Storage Layer

- Opened file: `users.adb` (AsteroidDB format).
- Read line 1: `{"id":1, "name":"Alice", "age":30, "role":"dev"}`.
- Read line 2: `{"id":2, "name":"Bob", "age":25, "role":"qa"}`.

#### Step 6: Output

The REPL prints:

```
| name  | age |
|-------|-----|
| Alice | 30  |
| Bob   | 25  |
```

---

## 3. Implementation Roadmap

### Phase 1: The Basics

- [ ] **Token Types**: Define enum for `SELECT`, `FROM`, `INSERT`, `VALUES`, `IDENTIFIER`, `LITERAL`.
- [ ] **Tokenizer**: Implement `std::string` -> `std::vector<Token>`.
- [ ] **AST Nodes**: Create structs for `Statement`, `SelectStatement`, `InsertStatement`.

### Phase 2: Storage & Execution

- [ ] **File Format**: Define a simple text-based format (e.g., CSV or JSON) for easy debugging.
- [ ] **Table Registry**: Map table names like `users` to file paths like `./data/users.json`.
- [ ] **Simple Select**: readable table scan.

### Phase 3: Writing Data

- [ ] **Insert Parser**: Handle `INSERT INTO table VALUES (...)`.
- [ ] **Append Operation**: Append new lines to the table file.

## 4. Technical Constraints

- **Language**: C++ (implied by `main.cpp`).
- **No External deps**: Keep it standard library only if possible (or minimal deps).
- **Concurrency**: Single-threaded for v1.
