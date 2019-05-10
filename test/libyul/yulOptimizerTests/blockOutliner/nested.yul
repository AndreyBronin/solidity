{
  {
    function f() -> x { x := 1 }
    { mstore(f(), 2) }
    { mstore(f(), 2) }
  }
}
// ====
// step: blockOutliner
// ----
// {
//     {
//         { _1() }
//         { _1() }
//     }
//     function f() -> x
//     { x := 1 }
//     function _1()
//     { mstore(f(), 2) }
// }
