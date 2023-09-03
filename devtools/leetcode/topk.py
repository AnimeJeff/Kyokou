
class Solution:
    @staticmethod
    def topKFrequent(nums, k: int):
        freq = [[] for _ in range(len(nums))]
        out = []
        for n in set(nums):
            freq[nums.count(n)].append(n)
        j = 0
        l = len(freq)-1
        while j < k:
            if freq[l]:
                out +=freq[l]
                j+=1
            l-=1
        print(out)
        return out

Solution.topKFrequent([1, 1, 1, 2, 2, 3,4,4,5,5,5,5,5,6,6,6,6,],3)