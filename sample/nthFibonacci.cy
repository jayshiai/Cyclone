function fib(n:int):int{
    if n <= 1 return n

    return fib(n-1) + fib(n-2)
}


print("Input Number: ")
var n = input()
var result = fib(int(n))

print(string(result))