{
    for {} returndatasize() { mstore(8, 9) } {
        for {} returndatasize() { mstore(1, 2) } {
            mstore(4, 5)
            break
        }
        if mload(10) { continue }
        break
	}
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     for { } returndatasize() { mstore(8, 9) }
//     {
//         if returndatasize() { mstore(4, 5) }
//         if mload(10) { continue }
//         break
//     }
// }
