
Command line input:

encryptUtil [-n #] [-k keyfile]
-n # Number of threads to create
-k keyfile Path to file containing key

Limitations :

1. Maximum number of thread allowed is 50
2. Maximum key size 1024
3. Minimum input size is 3

Testcases tested on:

1. Regular input given as example for 1,2,4,8,32 thread.
4. Regular input given as example for 50 thread. //Invalid thread test case.

5. Rerunning the XOR operation on the output (ciphertext) to make sure the output is right.
6.Tested with a file of 1GB to make sure Output is proper.
7. Edge case testing of 0,1,2 bytes data.
8. Key larger than input testing
9. Tested with file size smaller than the input.