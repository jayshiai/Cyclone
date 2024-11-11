var arr : int[] = {3,2,1,6,5,4,9,8,7,10}

function printArray(arr:int[]){
    var arrSize = size(arr)
    for i = 0 to arrSize-1{
        print(string(arr[i]) + " ")
    }
    print("\n")
}


function bubbleSort(arr:int[]){
    var n = size(arr)
    print("Size: "+ string(n) + "\n")
    for i = 0 to n - 2{
        for j = 0 to n - i - 2 {
            if (arr[j] > arr[j+1]){
               
                var temp = arr[j+1]
                arr[j+1] = arr[j]
                arr[j] = temp
            }
        }
    }

    print("SORTED ARRAY: \n")
    printArray(arr)
}

bubbleSort(arr)
print("UNSORTED ARRAY: \n")
printArray(arr)
