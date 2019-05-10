{
  {
    function f() -> x { x := 1 }
    { { mstore(f(), 2) } }
    { { mstore(f(), 2) } }
  }
}
// ====
// step: blockOutliner
// ----
// {
//     {
//         { _2() }
//         { _2() }
//     }
//     function f() -> x
//     { x := 1 }
//     function _1()
//     { mstore(f(), 2) }
//     function _2()
//     { { _1() } }
// }
