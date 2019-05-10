{
	for {} returndatasize() { mstore(1, 2) } {
		mstore(4, 5)
		break
	}
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     if returndatasize() { mstore(4, 5) }
// }
