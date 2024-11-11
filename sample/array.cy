var arr : int[] = {1,2,3,4,5,6,7,8,9,10}

function printArray(arr:int[], size:int){
    for i = 0 to size-1{
        print(string(arr[i]) + " ")
    }
}


printArray(arr, 10)