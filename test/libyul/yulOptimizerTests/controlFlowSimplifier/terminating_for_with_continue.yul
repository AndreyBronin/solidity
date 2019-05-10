{
	for {} returndatasize() { mstore(1, 2) } {
        if calldatasize() { continue }
		mstore(4, 5)
		break
	}
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     for { } returndatasize() { mstore(1, 2) }
//     {
//         if calldatasize() { continue }
//         mstore(4, 5)
//         break
//     }
// }
