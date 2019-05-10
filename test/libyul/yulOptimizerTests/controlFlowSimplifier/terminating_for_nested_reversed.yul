{
    for {} returndatasize() { mstore(8, 9) } {
        for {} returndatasize() { mstore(1, 2) } {
            mstore(4, 5)
            continue
        }
        break
	}
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     if returndatasize()
//     {
//         for { } returndatasize() { mstore(1, 2) }
//         {
//             mstore(4, 5)
//             continue
//         }
//     }
// }
