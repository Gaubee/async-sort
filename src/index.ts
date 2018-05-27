import { ConsolePro } from 'console-pro';
const console = new ConsolePro();

const asyncsort = require('bindings')('asyncsort.node')

export type TypedArray = Int8Array
    | Uint8Array
    | Uint8ClampedArray
    | Int16Array
    | Uint16Array
    | Int32Array
    | Uint32Array
    | Float32Array
    | Float64Array;
export type NumberArray = TypedArray | Array<number>;
const TypedArrayConstructor = Object.getPrototypeOf(Float64Array);

function swapElement(arr: NumberArray, a: number, b: number) {
    const temp = arr[a];
    arr[a] = arr[b];
    arr[b] = temp;
}
//非递归的形式  
function QuickSortNonRecursive(arr: NumberArray) {
    const length = arr.length;
    //先存大再存小，取得时候就可以先取小再取大，此处的大小指的是数组索引  
    const lowHigh = new Array(Math.ceil(Math.log2(length)) * 2);// 创建一个比较理想的大小，使用low_height_len能动态开拓大小
    // const lowHigh = new Uint32Array(length);
    let low_height_len = 0;
    lowHigh[low_height_len++] = length - 1;//push
    lowHigh[low_height_len++] = 0;//push
    let low: number;
    let high: number;
    // let _max_low_height_len = 0;
    while (low_height_len !== 0) {
        low = lowHigh[--low_height_len];//pop
        high = lowHigh[--low_height_len];//pop

        let i = low;
        let j = high;
        const value = arr[low];
        while (true) {

            while (arr[j] >= value && j > 0)
                j--;//右边的都大于等于value  
            if (j <= i) {
                break;
            }
            swapElement(arr, j, i);
            while (arr[i] <= value && i < j)
                i++;//左边的都小于等于value  
            if (i >= j) {
                break;
            }
            swapElement(arr, i, j);
        }
        //开始存储左右两侧待处理的数据，为了先处理左侧先保存右侧数据  
        const right_next_high = i + 1;
        if (right_next_high < high) {
            lowHigh[low_height_len++] = high;//push
            lowHigh[low_height_len++] = right_next_high;//push
        }
        //左侧  
        const left_next_low = i - 1;
        if (low < left_next_low) {
            lowHigh[low_height_len++] = left_next_low;//push
            lowHigh[low_height_len++] = low;//push
        }
        // if (_max_low_height_len < low_height_len) {
        //     _max_low_height_len = low_height_len
        //     console.flag("low_height_len", low_height_len, lowHigh.length, arr.length)
        // }
    }

}
export function sortBigNumberArray_1(arr: NumberArray) {
    const typed_arr = arr instanceof TypedArrayConstructor
        ? arr.slice() as TypedArray
        : Float64Array.from(arr);
    typed_arr.length > 1 && QuickSortNonRecursive(typed_arr);
    return typed_arr;
}
export function sortBigNumberArray_2(arr: NumberArray) {
    arr.length > 1 && QuickSortNonRecursive(arr);
    return arr;
}
export const sortBigNumberArray_3 = asyncsort.numberSort;
export function sortBigNumberArray(arr: NumberArray) {
    // const g = console.group(arr.join());
    const indexs = new Uint32Array(arr.length);
    let typed_arr = arr instanceof TypedArrayConstructor
        ? arr.slice() as TypedArray
        : Float64Array.from(arr);

    // Init left and right arrays.
    const leftArray = new Float64Array(arr.length);
    let left_arr_size = 0;
    const rightArray = new Float64Array(arr.length);
    let right_arr_size = 0;
    // Take the first element of array as a pivot.
    const pivotElement = typed_arr[0];
    typed_arr = typed_arr.subarray(1);//shift
    const centerArray = new Float64Array(arr.length);
    centerArray[0] = pivotElement;
    let center_arr_size = 1;
    // Split all array elements between left, center and right arrays.
    while (typed_arr.length) {
        const currentElement = typed_arr[0];
        typed_arr = typed_arr.subarray(1);//shift

        if (currentElement === pivotElement) {
            centerArray[center_arr_size++] = currentElement;
        } else if (currentElement < pivotElement) {
            leftArray[left_arr_size++] = currentElement;
        } else {
            rightArray[right_arr_size++] = currentElement;
        }
    }

    // Sort left and right arrays.
    const leftArraySorted = left_arr_size <= 1
        ? leftArray
        : sortBigNumberArray(leftArray.subarray(0, left_arr_size));
    const rightArraySorted = right_arr_size <= 1
        ? rightArray
        : sortBigNumberArray(rightArray.subarray(0, right_arr_size));

    // Let's now join sorted left array with center array and with sorted right array.
    const res = new Float64Array(arr.length);
    res.set(leftArraySorted.subarray(0, left_arr_size), 0);
    res.set(centerArray.subarray(0, center_arr_size), left_arr_size);
    res.set(rightArraySorted.subarray(0, right_arr_size), center_arr_size + left_arr_size);
    // console.flag('left', leftArray)
    // console.flag('center', centerArray)
    // console.flag('right', rightArray)
    // console.groupEnd(g);
    return res;//leftArraySorted.concat(centerArray, rightArraySorted);
}
if (require.main === module) {
    debugger
    const arr = Array.from({ length: 100000 }).map((_, i) => Math.random());
    // const arr = [25, 40, 54, 12, 10, 67, 98, 29, 46, 40];
    // console.log(arr)
    sortBigNumberArray_1(arr)
    //console.log();
    // console.log(sortBigNumberArray_1([4, 3, 2, 1, 5, 6, 7, 8]));
    // console.log(sortBigNumberArray_1([3, 1, 2]));
    console.log("zz")
    var res = asyncsort.numberSort([3, 2, 1, 6, 5, 4]);
    console.log(res);
    console.log("end")
}