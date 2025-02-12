# the/parser
The programming language parser

## Installation

```bash
the install the/parser
```

## Example usage

```the
import parse from "the/parser"
```

## API

### `interconnect () void`
Populates parent, next, prev properties with references.

**Return value**

none

**Examples**

```the
mut f := parse("./path")
interconnect(ref f)
```

### `parse (pathOrCode: str) File`
Parses path into `File` object.

**Parameters**

- `pathOrCode` - path or code to parse

**Return value**

Parsed path as `File` object.

**Exceptions**

- `ParserError` - thrown if path doesn't exists
- `ParserError` - thrown if path is not a file

**Examples**

```the
f := parse("path/to/project")
```

### `report (file: ref File) void`
Reports errors to `stderr` for the `File` object.

**Parameters**

- `file` - reference to `File` object

**Return value**

none

**Examples**

```the
f := parse("path/to/project")
report(ref f)
```

### `Program_traverse (program: ref Program, cb: (it: Statement) -> void) void`
Deep iterate through all of the program's statements.

**Parameters**

- `program` - reference to `Program` object to iterate through
- `cb` - callback that should be called for each statement

**Return value**

none

**Examples**

```the
f := parse("path/to/project")

Program_traverse(ref f.program, (it: Statement) -> void {
  Statement_toText(it)
})
```

### `Reader_init (pathOrCode: str) Reader`
Initializes reader instance with either path or code provided.

**Parameters**

- `pathOrCode` - path or code to initialize reader with.

**Return value**

A new reader instance.

**Examples**

```the
reader := Reader_init("/path/to/file")
reader := Reader_init("const PI := 3.14159")
```

**Exceptions**

- `ParserError` - thrown if path doesn't exists
- `ParserError` - thrown if path is not a file

### `Reader_error (reader: ref Reader, errorType: str, message: str, start: int, end: int) str`
Generates an error of specified type with specified message at specified position range.

**Parameters**

- `reader` - reader instance to retrieve lines information from.
- `errorType` - error type that goes before message.
- `message` - error message that is present on the first line.
- `start` - beginning of error location range.
- `end` - ending of error location range.

**Return value**

Generated error string.

**Examples**

```the
error := Reader_error(ref reader, "TypeError", "Missing type", 1, 10)
```

### `Reader_locate (reader: ref Reader, pos: int) Location`
Converts file position into location object.

**Parameters**

- `reader` - reader instance to locate position from.
- `pos` - position to locate.

**Return value**

File position converted into location object.

**Examples**

```the
loc := Reader_locate(ref reader, 10)
```

### `Expression_toText (self: Expression, indent := 0) str`
Generates string representation of the expression.

**Parameters**

- `self` - expression to generate string representation for
- `indent` - indentation for string representation

**Return value**

String representation of the expression.

**Examples**

```the
Expression_toText(expression)
Expression_toText(expression, indent: 2)
```

### `FunctionParameter_toText (self: FunctionParameter, indent := 0) str`
Generates string representation of the function parameter.

**Parameters**

- `self` - function parameter to generate string representation for
- `indent` - indentation for string representation

**Return value**

String representation of the function parameter.

**Examples**

```the
FunctionParameter_toText(param)
FunctionParameter_toText(param, indent: 2)
```

### `Identifier_toText (self: Identifier, indent := 0) str`
Generates string representation of the identifier.

**Parameters**

- `self` - identifier to generate string representation for
- `indent` - indentation for string representation

**Return value**

String representation of the identifier.

**Examples**

```the
Identifier_toText(id)
Identifier_toText(id, indent: 2)
```

### `Statement_toText (self: Statement, indent := 0) str`
Generates string representation of the statement.

**Parameters**

- `self` - statement to generate string representation for
- `indent` - indentation for string representation

**Return value**

String representation of the statement.

**Examples**

```the
Statement_toText(statement)
Statement_toText(statement, indent: 2)
```

### `Type_toText (self: Type, indent := 0) str`
Generates string representation of the type.

**Parameters**

- `self` - type to generate string representation for
- `indent` - indentation for string representation

**Return value**

String representation of the type.

**Examples**

```the
Type_toText(type)
Type_toText(type, indent: 2)
```

### `Reader.eof () bool`
Checks whether reader reached end of file.

**Return value**

Whether reader reached end of file.

**Examples**

```the
reader.eof()
```

### `Reader.lookahead (check: char) bool`
Increases position if the next character in the stream matches specified character.

**Parameters**

- `check` - character to check for.

**Return value**

Whether next character in the stream matches specified character.

**Examples**

```the
reader.lookahead('a')
```

### `Reader.next () char`
Increase position and returns next character from the stream.

**Return value**

Next character from the stream.

**Examples**

```the
ch := reader.next()
```

**Exceptions**

- `Error` - thrown when tried reading on end of file

### `Reader.seek (pos: int) void`
Change reader position.

**Parameters**

- `pos` - new reader position.

**Return value**

none

**Examples**

```the
reader.seek(100)
```

### `Reader.slice (start: int, end: int) str`
Create a slice of the reader content and return it.

**Parameters**

- `start` - slice starting position.
- `end` - slice ending position.

**Return value**

Slice of the reader content.

**Examples**

```the
slice := reader.slice(1, 5)
```

### `Reader.walk (matcher: (char) -> bool) void`
Iterate over reader content with matcher function.

**Parameters**

- `matcher` - matcher function to iterate with.

**Return value**

none

**Examples**

```the
fn whitespaceWalker (ch: char) bool {
  return ch.isWhitespace
}

slice := reader.walk(whitespaceWalker)
```

### `Expression.asIdentifier () ref Identifier`
### `Expression.asBooleanLiteral () ref BooleanLiteral`
### `Expression.asCharacterLiteral () ref CharacterLiteral`
### `Expression.asFloatingPointLiteral () ref FloatingPointLiteral`
### `Expression.asIntegerLiteral () ref IntegerLiteral`
### `Expression.asNilLiteral () ref NilLiteral`
### `Expression.asStringLiteral () ref StringLiteral`
### `Expression.asArray () ref ArrayExpression`
### `Expression.asAs () ref AsExpression`
### `Expression.asAssignment () ref AssignmentExpression`
### `Expression.asAwait () ref AwaitExpression`
### `Expression.asBinary () ref BinaryExpression`
### `Expression.asCall () ref CallExpression`
### `Expression.asClosure () ref ClosureExpression`
### `Expression.asConditional () ref ConditionalExpression`
### `Expression.asElementAccess () ref ElementAccessExpression`
### `Expression.asIs () ref IsExpression`
### `Expression.asMap () ref MapExpression`
### `Expression.asMemberAccess () ref MemberAccessExpression`
### `Expression.asObject () ref ObjectExpression`
### `Expression.asParenthesized () ref ParenthesizedExpression`
### `Expression.asPropertyAccess () ref PropertyAccessExpression`
### `Expression.asReference () ref ReferenceExpression`
### `Expression.asUnary () ref UnaryExpression`
These methods are shortcuts for `as` expression.

**Return value**

Returns body casted to corresponding body type.

**Examples**

```the
expression.asIdentifier()
```

### `Expression.isIdentifier () bool`
### `Expression.isBooleanLiteral () bool`
### `Expression.isCharacterLiteral () bool`
### `Expression.isFloatingPointLiteral () bool`
### `Expression.isIntegerLiteral () bool`
### `Expression.isNilLiteral () bool`
### `Expression.isStringLiteral () bool`
### `Expression.isArray () bool`
### `Expression.isAs () bool`
### `Expression.isAssignment () bool`
### `Expression.isAwait () bool`
### `Expression.isBinary () bool`
### `Expression.isCall () bool`
### `Expression.isClosure () bool`
### `Expression.isConditional () bool`
### `Expression.isElementAccess () bool`
### `Expression.isIs () bool`
### `Expression.isMap () bool`
### `Expression.isMemberAccess () bool`
### `Expression.isObject () bool`
### `Expression.isParenthesized () bool`
### `Expression.isPropertyAccess () bool`
### `Expression.isReference () bool`
### `Expression.isUnary () bool`
These methods are shortcuts for `is` expression.

**Return value**

Checks whether expression's body contains corresponding type.

**Examples**

```the
expression.isIdentifier()
```

### `Expression.getAccess (withAssignment := false) bool`
Finds `Identifier`, `ElementAccess` or `PropertyAccess` expression skipping parenthesized expression.

**Parameters**

- `withAssignment` - whether to treat left side of assignment as access

**Return value**

Whether expression is `Identifier`, `ElementAccess` or `PropertyAccess`.

**Examples**

```the
expression.getAccess()
expression.getAccess(withAssignment: true)
```

**Exceptions**

- `Error` - thrown if unable to get access expression

### `Expression.hasParent () bool`
Checks if expression has parent. Make sure to `interconnect` references.

**Return value**

Whether expression has parent expression.

**Examples**

```the
expression.hasParent()
```

### `Expression.hasParentNode () bool`
Checks if expression has parent statement. Make sure to `interconnect` references.

**Return value**

Whether expression has parent statement.

**Examples**

```the
expression.hasParentNode()
```

### `Expression.isAccess (withAssignment := false) bool`
Checks whether expression is `Identifier`, `ElementAccess` or `PropertyAccess`.

**Parameters**

- `withAssignment` - whether to treat left side of assignment as access

**Return value**

Whether expression is `Identifier`, `ElementAccess` or `PropertyAccess`.

**Examples**

```the
expression.isAccess()
expression.isAccess(withAssignment: true)
```

### `Expression.parent () ref Expression`
Return expression's parent. Make sure to `interconnect` references.

**Return value**

Expression's parent.

**Examples**

```the
expression.parent()
```

**Exceptions**

- `ParserError` - thrown if tried accessing nil parent

### `Expression.parentNode () ref Expression`
Return expression's parent statement. Make sure to `interconnect` references.

**Return value**

Expression's parent statement.

**Examples**

```the
expression.parentNode()
```

**Exceptions**

- `ParserError` - thrown if tried accessing nil parent statement

### `Statement.asBlock () ref BlockStatement`
### `Statement.asBreak () ref BreakStatement`
### `Statement.asContinue () ref ContinueStatement`
### `Statement.asEmpty () ref EmptyStatement`
### `Statement.asEnumDeclaration () ref EnumDeclaration`
### `Statement.asEof () ref EofStatement`
### `Statement.asExportDeclaration () ref ExportDeclaration`
### `Statement.asExportNamedDeclaration () ref ExportNamedDeclaration`
### `Statement.asExpression () ref ExpressionStatement`
### `Statement.asFunctionDeclaration () ref FunctionDeclaration`
### `Statement.asIf () ref IfStatement`
### `Statement.asImportDeclaration () ref ImportDeclaration`
### `Statement.asLoop () ref LoopStatement`
### `Statement.asMainDeclaration () ref MainDeclaration`
### `Statement.asObjectDeclaration () ref ObjectDeclaration`
### `Statement.asObjectDeclarationMethod () ref ObjectDeclarationMethod`
### `Statement.asObjectDeclarationProperty () ref ObjectDeclarationProperty`
### `Statement.asReturn () ref ReturnStatement`
### `Statement.asThrow () ref ThrowStatement`
### `Statement.asTry () ref TryStatement`
### `Statement.asTypeAliasDeclaration () ref TypeAliasDeclaration`
### `Statement.asVariableDeclaration () ref VariableDeclaration`
These methods are shortcuts for `as` expression.

**Return value**

Returns body casted to corresponding body type.

**Examples**

```the
statement.asBlock()
```

### `Statement.isBlock () bool`
### `Statement.isBreak () bool`
### `Statement.isContinue () bool`
### `Statement.isEmpty () bool`
### `Statement.isEnumDeclaration () bool`
### `Statement.isEof () bool`
### `Statement.isExportDeclaration () bool`
### `Statement.isExportNamedDeclaration () bool`
### `Statement.isExpression () bool`
### `Statement.isFunctionDeclaration () bool`
### `Statement.isIf () bool`
### `Statement.isImportDeclaration () bool`
### `Statement.isLoop () bool`
### `Statement.isMainDeclaration () bool`
### `Statement.isObjectDeclaration () bool`
### `Statement.isObjectDeclarationMethod () bool`
### `Statement.isObjectDeclarationProperty () bool`
### `Statement.isReturn () bool`
### `Statement.isThrow () bool`
### `Statement.isTry () bool`
### `Statement.isTypeAliasDeclaration () bool`
### `Statement.isVariableDeclaration () bool`
These methods are shortcuts for `is` expression.

**Return value**

Checks whether statement's body contains corresponding type.

**Examples**

```the
statement.isBlock()
```

### `Statement.hasAsyncParent () bool`
Check whether statement is placed inside async parent.

**Return value**

Whether statement is placed inside async parent.

**Examples**

```the
statement.hasAsyncParent()
```

### `Statement.hasLoopParent () bool`
Check whether statement is placed inside loop statement.

**Return value**

Whether statement is placed inside loop statement.

**Examples**

```the
statement.hasLoopParent()
```

### `Statement.hasMainParent () bool`
Check whether statement is placed inside main declaration.

**Return value**

Whether statement is placed inside a main declaration.

**Examples**

```the
statement.hasMainParent()
```

### `Statement.hasNext () bool`
Check if statement has next sibling. Make sure to `interconnect` references.

**Return value**

Whether statement has next sibling.

**Examples**

```the
statement.hasNext()
```

### `Statement.hasParent () bool`
Check if statement has parent. Make sure to `interconnect` references.

**Return value**

Whether statement has parent.

**Examples**

```the
statement.hasParent()
```

### `Statement.hasPrev () bool`
Check if statement has previous sibling. Make sure to `interconnect` references.

**Return value**

Whether statement has previous sibling.

**Examples**

```the
statement.hasPrev()
```

### `Statement.isDeclaration () bool`
Checks whether statements is declaration.

**Return value**

Whether statements is declaration.

**Examples**

```the
statement.isDeclaration()
```

### `Statement.isDeadEnd (allowLoopStatements := true) bool`
Checks whether statements is dead end.

**Parameters**

- `allowLoopStatements` - whether `break` and `continue` statements should count for dead end.

**Return value**

Whether statements is dead end.

**Examples**

```the
statement.isDeadEnd()
```

### `Statement.isLoopDeep () bool`
Checks whether statement is loop, recursively skipping block statements.

**Return value**

Whether statement is loop, recursively skipping block statements.

**Examples**

```the
statement.isLoopDeep()
```

### `Statement.next () ref Statement`
Return statement's next sibling. Make sure to `interconnect` references.

**Return value**

Statement's next sibling.

**Examples**

```the
statement.next()
```

**Exceptions**

- `ParserError` - thrown if tried accessing nil next

### `Statement.parent () ref Statement`
Return statement's parent. Make sure to `interconnect` references.

**Return value**

Statement's parent.

**Examples**

```the
statement.parent()
```

**Exceptions**

- `ParserError` - thrown if tried accessing nil parent

### `Statement.parentMaybe () (ref Statement)?`
Return optional statement's parent. Usually parent is `nil` when parent is within the root scope. Make sure to `interconnect` references.

**Return value**

Optional statement's parent.

**Examples**

```the
statement.parentMaybe()
```

### `Statement.prev () ref Statement`
Return statement's previous sibling. Make sure to `interconnect` references.

**Return value**

Statement's previous sibling.

**Examples**

```the
statement.prev()
```

**Exceptions**

- `ParserError` - thrown if tried accessing nil prev

### `Type.asArray () ref ArrayType`
### `Type.asFunction () ref FunctionType`
### `Type.asIdentifier () ref IdentifierType`
### `Type.asMap () ref MapType`
### `Type.asMember () ref MemberType`
### `Type.asOptional () ref OptionalType`
### `Type.asParenthesized () ref ParenthesizedType`
### `Type.asReference () ref ReferenceType`
### `Type.asUnion () ref UnionType`
These methods are shortcuts for `as` expression.

**Return value**

Returns body casted to corresponding body type.

**Examples**

```the
type.asArray()
```

### `Type.isArray () bool`
### `Type.isFunction () bool`
### `Type.isIdentifier () bool`
### `Type.isMap () bool`
### `Type.isMember () bool`
### `Type.isOptional () bool`
### `Type.isParenthesized () bool`
### `Type.isReference () bool`
### `Type.isUnion () bool`
These methods are shortcuts for `is` expression.

**Return value**

Checks whether type's body contains corresponding type.

**Examples**

```the
type.isArray()
```

### `expressionToType (self: Expression) Type`
Converts expression to type.

**Parameters**

- `self` - expression to convert to type.

**Return value**

Type representation of the expression.

**Examples**

```the
expressionToType(expression)
```

**Exceptions**

- `Error` - thrown if unable to transform

### `getInnerExpressions (self: ref Expression) (ref Expression)[]`
Get first level inner expressions of a specified expression.

**Parameters**

- `self` - expression to get inner expression from.

**Return value**

First level inner expressions.

**Examples**

```the
result := getInnerExpression(ref expression)
```

**Exceptions**

- `Error` - thrown if unable to get inner expressions

### `stringifyExpression (expression: Expression) str`
Converts expression to a code string.

**Parameters**

- `expression` - expression to convert to code string.

**Return value**

Code string representation of the expression.

**Examples**

```the
stringifyExpression(expression)
```

**Exceptions**

- `Error` - thrown if unable to convert to a code string

### `stringifyStatement (self: Statement) str`
Converts statement to a code string.

**Parameters**

- `self` - statement to convert to code string.

**Return value**

Code string representation of the statement.

**Examples**

```the
stringifyStatement(statement)
```

**Exceptions**

- `Error` - thrown if unable to convert to a code string

### `stringifyType (self: Type) str`
Converts type to a code string.

**Parameters**

- `self` - type to convert to code string.

**Return value**

Code string representation of the type.

**Examples**

```the
stringifyType(type)
```

**Exceptions**

- `Error` - thrown if unable to convert to a code string

### `validObjExprId (expression: Expression) bool`
Checks whether expression is valid `ObjectExpression` identifier.

**Parameters**

- `expression` - expression to check.

**Return value**

Whether expression is valid `ObjectExpression` identifier.

**Examples**

```the
validObjExprId(expression)
```

### `typeToExpression (type: Type) Expression`
Converts type to expression.

**Parameters**

- `type` - type to convert to expression.

**Return value**

Expression representation of the type.

**Examples**

```the
typeToExpression(type)
```

**Exceptions**

- `Error` - thrown if unable to transform
