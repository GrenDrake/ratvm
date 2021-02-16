# Rat Code Language

RatCode is the language created for RatVM.
Its design draws inspiration from many sources and was mostly built using the "throw stuff together and see what works" technique.

RatCode files typically have the extension `.ratc`.

## Comments

RatCode supports both C-style comments (`/* ... */`) and C++ style comments. (`// ...`).
C-style comments may not be nested; doing so will produce a compiler error.

## Value Types

Every value in RatCode is of one of a handful of types.
Conversions between types is never performed automatically; the `astype` opcode can cast a value from one type to another, but this can have unexpected effects if used carelessly.

There are two types of value: reference values refer to data contained elsewhere (such as a value referring to an object) and primitive values directly contain the value's data in the value (such as with integers).
String, List, Map, Function, and Object types are always reference values.

**Any**:
The Any type is not an actual type, but rather a placeholder in argument lists that permits any type of value to be passed.

**None**:
The None type represents a lack of a value and is largely equivalent to `null` in other languages.
The None type has only one valid valid, called `none`.

**Integer**:
Integers in RatCode are signed, 32-bit values.
Valid values are whole numbers in the range 2,147,483,648 to 2,147,483,647.

**Property**:
Property names are identifiers prefixed with a dollar sign (`$`) that serve to identify specific properties on objects.
They exist within their own namespace and are represented internally as integers.

**Type**:
Types are the names of the various types of value used in RatCode.
The valid entries are the same as the headings found in this list.

**Vocab**:
Vocab values are vocabulary that can be compared to text the player types into the game.

**Reference**:
Used only with specific opcodes, References values are typically created automatically and can be mostly ignored in normal development.
A Reference value contains a reference to a specific local variable of the current function.

**String**:
A value that references a specific string in the game's string database.

**List**:
A value that references a specific list in the game's list database.

**Map**:
A value that references a specific map in the game's map database.

**Function**:
A value that references a specific function in the game's function database.

**Object**:
A value that references a specific object in the game's object database.


## Declarations

At the top level, a rat-code source file consists of a series of declaration.
Significantly, these are *not* variables; the value of a declaration can never change.
For most data forms, this makes use of the `declare` keyword:

```
declare anInteger 7384;
declare aString "A Name";
```

More complex data structures can also be declared, such as lists and maps.
Note that the immutability of a declaration does not apply to the contents of a list or map.
The declare name will always refer to the same list or map, but items can be added, removed, or changed within it.

```
declare aList [ 1 2 3 ];
declare aMap { 1: "The" 2: "End" 3: "Is" 4: "Far" };
```

RatCode also defines a special datatype: the flagset.
The values listed in a flagset are bitwise-ored together during compilation to produce a single integer.

```
declare testFlags flags( 1 4 32 declaredInteger );
```

Finally, there are two complex data types that have their own declaration syntax: functions and objects.
They may not be declared using the `declare` keyword.

### Functions

Functions are declared using the `function` keyword.
They may take multiple arguments, each of which can have a type specified.
When not specified, the type defaults to Any.
The types of the values passed to a function are checked at run-time.

The syntax of statements inside a function is covered later.

```
function aFunction(arg1 arg2:Integer) {
    ("Arguments: " arg1 arg2 "\n")
    (return arg2)
}
```

### Objects

Objects are declared using the `object` keyword.
The body of an object consists of a series of property-value pairs.

Property names always begin with a dollar sign (`$`) and thus exist in their own namespace.
As such, there is no concern about property names being shadowed by other names.

Values may be any valid declaration value, or may be an anonymous function or object.

```
object anObject
    $firstProperty function() { }
    $secondProperty 826
    $thirdProperty object subObject;
;
```
