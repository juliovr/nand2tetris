@256                   // L-0
D=A                    // L-1
@SP                    // L-2
M=D                    // L-3

// call Sys.init 0
@return_0                 // L-4
D=A                        // L-5
@SP                        // L-6
A=M                        // L-7
M=D                        // L-8
@SP                        // L-9
M=M+1                      // L-10
@LCL                       // L-11
D=M                        // L-12
@SP                        // L-13
A=M                        // L-14
M=D                        // L-15
@SP                        // L-16
M=M+1                      // L-17
@ARG                       // L-18
D=M                        // L-19
@SP                        // L-20
A=M                        // L-21
M=D                        // L-22
@SP                        // L-23
M=M+1                      // L-24
@THIS                      // L-25
D=M                        // L-26
@SP                        // L-27
A=M                        // L-28
M=D                        // L-29
@SP                        // L-30
M=M+1                      // L-31
@THAT                      // L-32
D=M                        // L-33
@SP                        // L-34
A=M                        // L-35
M=D                        // L-36
@SP                        // L-37
M=M+1                      // L-38
@SP                        // L-39
D=M                        // L-40
@5                        // L-41
D=D-A                      // L-42
@ARG                       // L-43
M=D                        // L-44
@SP                        // L-45
D=M                        // L-46
@LCL                       // L-47
M=D                        // L-48
@Sys.init                        // L-49
0;JMP                      // L-50
(return_0)

// end call statement

// function Sys.init 0
(Sys.init)

// end function declaration
@4000                    // L-51
D=A                    // L-52
@SP                    // L-53
A=M                    // L-54
M=D                    // L-55
@SP                    // L-56
M=M+1                  // L-57
@SP                      // L-58
M=M-1                    // L-59
A=M                      // L-60
D=M                      // L-61
@THIS                    // L-62
M=D                      // L-63
@5000                    // L-64
D=A                    // L-65
@SP                    // L-66
A=M                    // L-67
M=D                    // L-68
@SP                    // L-69
M=M+1                  // L-70
@SP                      // L-71
M=M-1                    // L-72
A=M                      // L-73
D=M                      // L-74
@THAT                    // L-75
M=D                      // L-76

// call Sys.main 0
@return_1                 // L-77
D=A                        // L-78
@SP                        // L-79
A=M                        // L-80
M=D                        // L-81
@SP                        // L-82
M=M+1                      // L-83
@LCL                       // L-84
D=M                        // L-85
@SP                        // L-86
A=M                        // L-87
M=D                        // L-88
@SP                        // L-89
M=M+1                      // L-90
@ARG                       // L-91
D=M                        // L-92
@SP                        // L-93
A=M                        // L-94
M=D                        // L-95
@SP                        // L-96
M=M+1                      // L-97
@THIS                      // L-98
D=M                        // L-99
@SP                        // L-100
A=M                        // L-101
M=D                        // L-102
@SP                        // L-103
M=M+1                      // L-104
@THAT                      // L-105
D=M                        // L-106
@SP                        // L-107
A=M                        // L-108
M=D                        // L-109
@SP                        // L-110
M=M+1                      // L-111
@SP                        // L-112
D=M                        // L-113
@5                        // L-114
D=D-A                      // L-115
@ARG                       // L-116
M=D                        // L-117
@SP                        // L-118
D=M                        // L-119
@LCL                       // L-120
M=D                        // L-121
@Sys.main                        // L-122
0;JMP                      // L-123
(return_1)

// end call statement
@SP                      // L-124
M=M-1                    // L-125
A=M                      // L-126
D=M                      // L-127
@R6                     // L-128
M=D                      // L-129
(LOOP)
@LOOP                        // L-130
0;JMP                      // L-131

// function Sys.main 5
(Sys.main)
@0                       // L-132
D=A                      // L-133
@SP                      // L-134
A=M                      // L-135
M=D                      // L-136
@SP                      // L-137
M=M+1                    // L-138
@0                       // L-139
D=A                      // L-140
@SP                      // L-141
A=M                      // L-142
M=D                      // L-143
@SP                      // L-144
M=M+1                    // L-145
@0                       // L-146
D=A                      // L-147
@SP                      // L-148
A=M                      // L-149
M=D                      // L-150
@SP                      // L-151
M=M+1                    // L-152
@0                       // L-153
D=A                      // L-154
@SP                      // L-155
A=M                      // L-156
M=D                      // L-157
@SP                      // L-158
M=M+1                    // L-159
@0                       // L-160
D=A                      // L-161
@SP                      // L-162
A=M                      // L-163
M=D                      // L-164
@SP                      // L-165
M=M+1                    // L-166

// end function declaration
@4001                    // L-167
D=A                    // L-168
@SP                    // L-169
A=M                    // L-170
M=D                    // L-171
@SP                    // L-172
M=M+1                  // L-173
@SP                      // L-174
M=M-1                    // L-175
A=M                      // L-176
D=M                      // L-177
@THIS                    // L-178
M=D                      // L-179
@5001                    // L-180
D=A                    // L-181
@SP                    // L-182
A=M                    // L-183
M=D                    // L-184
@SP                    // L-185
M=M+1                  // L-186
@SP                      // L-187
M=M-1                    // L-188
A=M                      // L-189
D=M                      // L-190
@THAT                    // L-191
M=D                      // L-192
@200                    // L-193
D=A                    // L-194
@SP                    // L-195
A=M                    // L-196
M=D                    // L-197
@SP                    // L-198
M=M+1                  // L-199
@1                      // L-200
D=A                      // L-201
@LCL                     // L-202
D=M+D                    // L-203
@R13                     // L-204
M=D                      // L-205
@SP                      // L-206
M=M-1                    // L-207
A=M                      // L-208
D=M                      // L-209
@R13                     // L-210
A=M                      // L-211
M=D                      // L-212
@40                    // L-213
D=A                    // L-214
@SP                    // L-215
A=M                    // L-216
M=D                    // L-217
@SP                    // L-218
M=M+1                  // L-219
@2                      // L-220
D=A                      // L-221
@LCL                     // L-222
D=M+D                    // L-223
@R13                     // L-224
M=D                      // L-225
@SP                      // L-226
M=M-1                    // L-227
A=M                      // L-228
D=M                      // L-229
@R13                     // L-230
A=M                      // L-231
M=D                      // L-232
@6                    // L-233
D=A                    // L-234
@SP                    // L-235
A=M                    // L-236
M=D                    // L-237
@SP                    // L-238
M=M+1                  // L-239
@3                      // L-240
D=A                      // L-241
@LCL                     // L-242
D=M+D                    // L-243
@R13                     // L-244
M=D                      // L-245
@SP                      // L-246
M=M-1                    // L-247
A=M                      // L-248
D=M                      // L-249
@R13                     // L-250
A=M                      // L-251
M=D                      // L-252
@123                    // L-253
D=A                    // L-254
@SP                    // L-255
A=M                    // L-256
M=D                    // L-257
@SP                    // L-258
M=M+1                  // L-259

// call Sys.add12 1
@return_2                 // L-260
D=A                        // L-261
@SP                        // L-262
A=M                        // L-263
M=D                        // L-264
@SP                        // L-265
M=M+1                      // L-266
@LCL                       // L-267
D=M                        // L-268
@SP                        // L-269
A=M                        // L-270
M=D                        // L-271
@SP                        // L-272
M=M+1                      // L-273
@ARG                       // L-274
D=M                        // L-275
@SP                        // L-276
A=M                        // L-277
M=D                        // L-278
@SP                        // L-279
M=M+1                      // L-280
@THIS                      // L-281
D=M                        // L-282
@SP                        // L-283
A=M                        // L-284
M=D                        // L-285
@SP                        // L-286
M=M+1                      // L-287
@THAT                      // L-288
D=M                        // L-289
@SP                        // L-290
A=M                        // L-291
M=D                        // L-292
@SP                        // L-293
M=M+1                      // L-294
@SP                        // L-295
D=M                        // L-296
@6                        // L-297
D=D-A                      // L-298
@ARG                       // L-299
M=D                        // L-300
@SP                        // L-301
D=M                        // L-302
@LCL                       // L-303
M=D                        // L-304
@Sys.add12                        // L-305
0;JMP                      // L-306
(return_2)

// end call statement
@SP                      // L-307
M=M-1                    // L-308
A=M                      // L-309
D=M                      // L-310
@R5                     // L-311
M=D                      // L-312
@0                    // L-313
D=A                    // L-314
@LCL                   // L-315
A=M+D                  // L-316
D=M                    // L-317
@SP                    // L-318
A=M                    // L-319
M=D                    // L-320
@SP                    // L-321
M=M+1                  // L-322
@1                    // L-323
D=A                    // L-324
@LCL                   // L-325
A=M+D                  // L-326
D=M                    // L-327
@SP                    // L-328
A=M                    // L-329
M=D                    // L-330
@SP                    // L-331
M=M+1                  // L-332
@2                    // L-333
D=A                    // L-334
@LCL                   // L-335
A=M+D                  // L-336
D=M                    // L-337
@SP                    // L-338
A=M                    // L-339
M=D                    // L-340
@SP                    // L-341
M=M+1                  // L-342
@3                    // L-343
D=A                    // L-344
@LCL                   // L-345
A=M+D                  // L-346
D=M                    // L-347
@SP                    // L-348
A=M                    // L-349
M=D                    // L-350
@SP                    // L-351
M=M+1                  // L-352
@4                    // L-353
D=A                    // L-354
@LCL                   // L-355
A=M+D                  // L-356
D=M                    // L-357
@SP                    // L-358
A=M                    // L-359
M=D                    // L-360
@SP                    // L-361
M=M+1                  // L-362
@SP                    // L-363
M=M-1                  // L-364
A=M                    // L-365
D=M                    // L-366
@SP                    // L-367
M=M-1                  // L-368
A=M                    // L-369
M=M+D                  // L-370
@SP                    // L-371
M=M+1                  // L-372
@SP                    // L-373
M=M-1                  // L-374
A=M                    // L-375
D=M                    // L-376
@SP                    // L-377
M=M-1                  // L-378
A=M                    // L-379
M=M+D                  // L-380
@SP                    // L-381
M=M+1                  // L-382
@SP                    // L-383
M=M-1                  // L-384
A=M                    // L-385
D=M                    // L-386
@SP                    // L-387
M=M-1                  // L-388
A=M                    // L-389
M=M+D                  // L-390
@SP                    // L-391
M=M+1                  // L-392
@SP                    // L-393
M=M-1                  // L-394
A=M                    // L-395
D=M                    // L-396
@SP                    // L-397
M=M-1                  // L-398
A=M                    // L-399
M=M+D                  // L-400
@SP                    // L-401
M=M+1                  // L-402

// beginning return
@LCL                       // L-403
D=M                        // L-404
@R12                       // L-405
M=D                        // L-406
@5                         // L-407
A=D-A                      // L-408
D=M                        // L-409
@R15                       // L-410
M=D                        // L-411
@0                         // L-412
D=A                        // L-413
@ARG                       // L-414
D=M+D                      // L-415
@R13                       // L-416
M=D                        // L-417
@SP                        // L-418
M=M-1                      // L-419
A=M                        // L-420
D=M                        // L-421
@R13                       // L-422
A=M                        // L-423
M=D                        // L-424
@ARG                       // L-425
D=M                        // L-426
@SP                        // L-427
M=D+1                      // L-428
@R12                       // L-429
A=M-1                      // L-430
D=M                        // L-431
@THAT                      // L-432
M=D                        // L-433
@2                         // L-434
D=A                        // L-435
@R12                       // L-436
A=M-D                      // L-437
D=M                        // L-438
@THIS                      // L-439
M=D                        // L-440
@3                         // L-441
D=A                        // L-442
@R12                       // L-443
A=M-D                      // L-444
D=M                        // L-445
@ARG                       // L-446
M=D                        // L-447
@4                         // L-448
D=A                        // L-449
@R12                       // L-450
A=M-D                      // L-451
D=M                        // L-452
@LCL                       // L-453
M=D                        // L-454
@R15                       // L-455
A=M                        // L-456
0;JMP                      // L-457

// end return

// function Sys.add12 0
(Sys.add12)

// end function declaration
@4002                    // L-458
D=A                    // L-459
@SP                    // L-460
A=M                    // L-461
M=D                    // L-462
@SP                    // L-463
M=M+1                  // L-464
@SP                      // L-465
M=M-1                    // L-466
A=M                      // L-467
D=M                      // L-468
@THIS                    // L-469
M=D                      // L-470
@5002                    // L-471
D=A                    // L-472
@SP                    // L-473
A=M                    // L-474
M=D                    // L-475
@SP                    // L-476
M=M+1                  // L-477
@SP                      // L-478
M=M-1                    // L-479
A=M                      // L-480
D=M                      // L-481
@THAT                    // L-482
M=D                      // L-483
@0                    // L-484
D=A                    // L-485
@ARG                   // L-486
A=M+D                  // L-487
D=M                    // L-488
@SP                    // L-489
A=M                    // L-490
M=D                    // L-491
@SP                    // L-492
M=M+1                  // L-493
@12                    // L-494
D=A                    // L-495
@SP                    // L-496
A=M                    // L-497
M=D                    // L-498
@SP                    // L-499
M=M+1                  // L-500
@SP                    // L-501
M=M-1                  // L-502
A=M                    // L-503
D=M                    // L-504
@SP                    // L-505
M=M-1                  // L-506
A=M                    // L-507
M=M+D                  // L-508
@SP                    // L-509
M=M+1                  // L-510

// beginning return
@LCL                       // L-511
D=M                        // L-512
@R12                       // L-513
M=D                        // L-514
@5                         // L-515
A=D-A                      // L-516
D=M                        // L-517
@R15                       // L-518
M=D                        // L-519
@0                         // L-520
D=A                        // L-521
@ARG                       // L-522
D=M+D                      // L-523
@R13                       // L-524
M=D                        // L-525
@SP                        // L-526
M=M-1                      // L-527
A=M                        // L-528
D=M                        // L-529
@R13                       // L-530
A=M                        // L-531
M=D                        // L-532
@ARG                       // L-533
D=M                        // L-534
@SP                        // L-535
M=D+1                      // L-536
@R12                       // L-537
A=M-1                      // L-538
D=M                        // L-539
@THAT                      // L-540
M=D                        // L-541
@2                         // L-542
D=A                        // L-543
@R12                       // L-544
A=M-D                      // L-545
D=M                        // L-546
@THIS                      // L-547
M=D                        // L-548
@3                         // L-549
D=A                        // L-550
@R12                       // L-551
A=M-D                      // L-552
D=M                        // L-553
@ARG                       // L-554
M=D                        // L-555
@4                         // L-556
D=A                        // L-557
@R12                       // L-558
A=M-D                      // L-559
D=M                        // L-560
@LCL                       // L-561
M=D                        // L-562
@R15                       // L-563
A=M                        // L-564
0;JMP                      // L-565

// end return
