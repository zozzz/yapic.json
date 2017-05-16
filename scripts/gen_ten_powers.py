

if __name__ == "__main__":
    with open("exponents.h", "w") as f:
        f.write("/*** !!! AUTO GENERATED DO NOT EDIT !!! ***/\n\n")
        f.write("#ifndef EXPONENTS_H_\n")
        f.write("#define EXPONENTS_H_\n\n")

        max_pow = 20

        f.write("static const double pow_of_10[] = {")

        for x in range(max_pow):
            f.write(("10e+%s") % (x - 1))
            if x != max_pow - 1:
                f.write(", ")

        f.write("};\n\n")

        f.write("static const double sqr_of_10[] = {")

        for x in range(max_pow):
            f.write(("10e-%s") % (x + 1))
            if x != max_pow - 1:
                f.write(", ")

        f.write("};\n\n")

        f.write("#endif /* EXPONENTS_H_ */\n")
