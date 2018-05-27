// hello.cc using N-API
#include <node_api.h>
#include <assert.h>
#include <stack>
#include <map>
#define MAX_CANCEL_THREADS 6

typedef struct
{
	napi_ref _input;
	napi_ref _output;
	double _input_double;
	double _output_double;
	napi_ref _callback;
	napi_async_work _request;
} carrier;

carrier the_carrier;
carrier async_carrier[MAX_CANCEL_THREADS];

namespace async_sort
{

	void swapElement(double& a, double& b)
	{
		double temp = a;
		a = b;
		b = temp;
	}
	void swapIndex(uint32_t& a, uint32_t& b)
	{
		uint32_t temp = a;
		a = b;
		b = temp;
	}

	void QuickSortNonRecursive(double arr[], uint32_t indexs[], int length)
	{
		std::stack<uint32_t> lowHigh;//先存大再存小，取得时候就可以先取小再取大，此处的大小指的是数组索引  
		lowHigh.push(length - 1);
		lowHigh.push(0);
		uint32_t low, high;
		while (!lowHigh.empty())
		{
			low = lowHigh.top(); lowHigh.pop();
			high = lowHigh.top(); lowHigh.pop();

			uint32_t i = low;
			uint32_t j = high;
			double value = arr[low];
			while (true)
			{

				while (arr[j] >= value && j > 0)
					j--;//右边的都大于等于value
				if (j <= i) {
					break;
				}
				swapElement(arr[i], arr[j]);
				swapIndex(indexs[i], indexs[j]);
				while (arr[i] <= value && i < j)
					i++;//左边的都小于等于value
				if (i >= j) {
					break;
				}
				swapElement(arr[i], arr[j]);
				swapIndex(indexs[i], indexs[j]);
			}
			//开始存储左右两侧待处理的数据，为了先处理左侧先保存右侧数据  
			uint32_t right_next_high = i + 1;
			if (right_next_high < high) {
				lowHigh.push(high);
				lowHigh.push(right_next_high);
			}

			//左侧  
			if (i > 0) {
				uint32_t left_next_low = i - 1;
				if (low < left_next_low) {
					lowHigh.push(left_next_low);
					lowHigh.push(low);
				}
			}
		}
	}
	napi_value QuickSortNonRecursiveJsNumber(napi_env env, napi_callback_info info)
	{
		napi_status status;

		size_t argc = 2; // 接收两个参数，一个是数组，一个是回调
		napi_value args[2];
		// void* data;
		status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
		assert(status == napi_ok);
		napi_value arr = args[0];
		napi_value cb = args[1];

		napi_value global;
		status = napi_get_global(env, &global);
		assert(status == napi_ok);

		uint32_t arr_len;
		status = napi_get_array_length(env, arr, &arr_len);
		assert(status == napi_ok);
		double *arr_for_sort = new double[arr_len];
		uint32_t *indexs_for_sort = new uint32_t[arr_len];

		for (uint32_t i = 0; i < arr_len; i += 1)
		{
			napi_value item;
			status = napi_get_element(env, arr, i, &item);
			assert(status == napi_ok);

			double item_number;
			status = napi_get_value_double(env, item, &item_number);
			assert(status == napi_ok);
			arr_for_sort[i] = item_number;
			indexs_for_sort[i] = i;
		}

		QuickSortNonRecursive(arr_for_sort, indexs_for_sort, arr_len);

		napi_value cb_args[2];
		napi_get_null(env, &cb_args[0]); // no error

		//// 把结果导出成UInit32Array
		//napi_value indexs_array_buffer;
		//void* output_ptr = NULL;
		////  4 === Uint32Array.BYTES_PER_ELEMENT
		//status = napi_create_arraybuffer(env, arr_len * 4, &output_ptr, &indexs_array_buffer);
		//assert(status == napi_ok);
		//// uint32_t 与 size_t 本质上都是unsigned int，可以直接套用。napi_typedarray_type.napi_uint32_array
		//napi_value indexs_typed_array;
		//status = napi_create_typedarray(env, napi_uint32_array, arr_len, indexs_array_buffer, 0, &indexs_typed_array);
		//assert(status == napi_ok);
		//for (uint32_t i = 0; i < arr_len; i += 1) {
		//	napi_value index;
		//	status = napi_create_uint32(env, indexs_for_sort[i], &index);
		//	assert(status == napi_ok);
		//	status = napi_set_element(env, indexs_typed_array, i, index);
		//	assert(status == napi_ok);
		//}
		//cb_args[1] = indexs_typed_array;

		//napi_value result;
		//status = napi_call_function(env, global, cb, 2, cb_args, &result);
		//assert(status == napi_ok);

		//return result;

		// 整理排序结果
		//std::map<uint32_t,napi_value> tmp_map;
		napi_value tmp_arr;
		status = napi_create_array_with_length(env, arr_len, &tmp_arr);
		assert(status == napi_ok);
		for (uint32_t i = 0; i < arr_len; i += 1) {
			uint32_t new_value_index = indexs_for_sort[i];
			if (new_value_index == i) {// 没有位置变动，略过
				continue;
			}
			else if (new_value_index > i) {//来自后面的元素，把当前元素放入缓冲区，等后面来取
				napi_value tmp_item;
				status = napi_get_element(env, arr, i, &tmp_item);
				assert(status == napi_ok);
				status = napi_set_element(env, tmp_arr, i, tmp_item);
				assert(status == napi_ok);
				//tmp_map.insert(std::pair<uint32_t,napi_value>(i, tmp_item));
				// 更新元素
				napi_value new_item;
				status = napi_get_element(env, arr, new_value_index, &new_item);
				assert(status == napi_ok);
				status = napi_set_element(env, arr, i, new_item);
				assert(status == napi_ok);
			}
			else {// 从缓冲区中取前面缓存的元素
				napi_value tmp_item;
				status = napi_get_element(env, tmp_arr, new_value_index, &tmp_item);
				assert(status == napi_ok);
				status = napi_set_element(env, arr, i, tmp_item);
				assert(status == napi_ok);
			}
		}
		return arr;
	}
	napi_value init(napi_env env, napi_value exports)
	{
		napi_status status;

		napi_value fn;
		status = napi_create_function(env, nullptr, 0, QuickSortNonRecursiveJsNumber, nullptr, &fn);
		assert(status == napi_ok);
		status = napi_set_named_property(env, exports, "numberSort", fn);
		assert(status == napi_ok);

		return exports;
	}

	NAPI_MODULE(NODE_GYP_MODULE_NAME, init)

}
