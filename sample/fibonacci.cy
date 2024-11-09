function fib(n:int){
    if n <= 1{
       print("Invalid Input")
       return
    }

    var prev1 = 1
    var prev2 = 0

    for i = 1 to n
    {
        if i > 2{
            var current = prev1 + prev2
        prev2 = prev1
        prev1 = current
        print(string(current) + " ")
        continue
        }

        if i == 1{
            print("0 ")
            continue
        }

        if i == 2{
            print("1 ")
            continue
        }

    }
    print("\n")
}


print("Input Number: ")
var n = input()
fib(int(n))