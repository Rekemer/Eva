# Eva programming language
The experimental basic toy language is inspired by Clox from crafting interpreters, however Eva has AST representation of programm and types


### Example
```
g:= 2;

fun print( a : float, b: int)
{
	Print g;
	c :int = b + a;
	return c;
}

fun a () : int 
{	
	return 2;
}
fun main() : int
{
	Print g1;
	a := 2.0;
	print(1,a());
	return 0;
}
```