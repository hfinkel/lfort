/* RUN: cp %s %t
   RUN: %lfort_cc1 -x objective-c -fixit %t
   RUN: %lfort_cc1 -x objective-c -Werror %t
 */

// rdar://9036633

int main() {
  auto int i = 0;
  return i;
}
