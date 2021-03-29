
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* twoSum(int *nums, int numsSize, int target, int *returnSize) {
	int *ret;
	Map *map = map_init(10);

	*returnSize = 0;
	ret = (int *)malloc(sizeof(int) * 2);
	if (!ret) goto out;

	for (int i = 0; i < numsSize; i++) {
		int complement = target - nums[i];
		void *p = map_get(map, complement);

		if (p) { /* found in map */
			ret[0] = i;
			ret[1] = *((int *)p);
			*returnSize = 2;
			goto out;
		}

		p = malloc(sizeof(int));
		*((int *)p) = i;
		map_add(map, nums[i], p);
	}

out:
	map_deinit(map);
	return ret;
}

