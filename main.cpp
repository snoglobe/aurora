#include "context.h"
#include "std_lib.h"

int main() {
    AuroraContext context(R"(
        fn is_prime n
            if n < 2 return false
            for i, range(2, n)
                if n % i == 0 return false
            end
            return true
        end

        for i, range(1, 10000)
            if is_prime(i)
                print i, " is prime"
            else
                print i, " is not prime"
            end
        end
        )", globals);
    context.run();
    return 0;
}
