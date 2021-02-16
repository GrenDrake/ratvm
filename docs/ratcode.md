# Rat Code Language

RatCode is the language created for RatVM.
Its design draws inspiration from many sources and was mostly built using the "throw stuff together and see what works" technique.

RatCode files typically have the extension `.ratc`.

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

Functions are ddclared using the `function` keyword.
They may take multiple arguments, each of which can have a type specified.
When not specified, the type defauls to any.
The types of the values passed to a function are checked at run-time.

The syntax of statements insidd a function is covered later.

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
