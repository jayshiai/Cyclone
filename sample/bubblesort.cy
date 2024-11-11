var arr : int[] = {3,2,1,6,5,4,9,8,7,10}

function printArray(arr:int[], size:int){
    for i = 0 to size-1{
        print(string(arr[i]) + " ")
    }
    print("\n")
}


function bubbleSort(arr:int[], size: int){
    var swapped : bool

    for i = 0 to size - 2 {
        swapped = false

        for j = 0 to size - i - 2 {

            if (arr[j] > arr[j+1]){
                var temp:int = arr[j+1]
                arr[j+1] = arr[j]
                arr[j] = temp
                swapped = true
            }

        }

        if (!swapped){
            break
        }
    }

    print("SORTED ARRAY: \n")
    printArray(arr, size)
}

bubbleSort(arr, 10)
print("UNSORTED ARRAY: \n")
printArray(arr, 10)