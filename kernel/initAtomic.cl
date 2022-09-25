kernel void initAtomic_kernel(global int* atomicBuffer) {
	/*atomic_store_explicit((global atomic_int*)atomicBuffer, 0, memory_order_seq_cst);*/
	atomic_xchg(atomicBuffer, 0);
}