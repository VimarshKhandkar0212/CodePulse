def evaluate_expression(op1, operator, op2):
    op1 = float(op1) if '.' in op1 or 'e' in op1 else int(op1)
    op2 = float(op2) if '.' in op2 or 'e' in op2 else int(op2)
    if operator == '+': return op1 + op2
    if operator == '-': return op1 - op2
    if operator == '*': return op1 * op2
    if operator == '/': return op1 / op2
    raise ValueError(f"Unsupported operator: {operator}")

def get_value(token, variables):
    if token.startswith('"') and token.endswith('"'):
        return token.strip('"')  # String literal
    return variables.get(token, token)

def main():
    variables = {}
    call_stack = []
    return_value = None

    with open("tac.txt", "r") as f:
        lines = [line.strip() for line in f if line.strip()]

    for line in lines:
        print(f"Executing: {line}")

        if line.startswith("func "):
            print(f"Entering function {line.split()[1][:-1]}")
            continue

        elif line.startswith("endfunc"):
            print("Function end\n")
            continue

        elif line.startswith("param "):
            param = line.split(" ", 1)[1]
            val = get_value(param, variables)
            call_stack.append(val)
            continue

        elif line.startswith("call "):
            parts = line.split()
            func_name = parts[1].strip(",")
            arg_count = int(parts[2])

            args = call_stack[-arg_count:]
            call_stack = call_stack[:-arg_count]

            if func_name == "printf":
                print("Output:", *args)
            else:
                print(f"Calling {func_name} with args {args}")
            continue

        elif line.startswith("return "):
            ret = line.split()[1]
            return_value = get_value(ret, variables)
            print("Return value:", return_value)
            continue

        elif "=" in line:
            left, right = map(str.strip, line.split("="))
            tokens = right.split()

            if len(tokens) == 1:
                # Simple assignment or string
                val = get_value(tokens[0], variables)
                variables[left] = val
            elif len(tokens) == 3:
                op1 = get_value(tokens[0], variables)
                operator = tokens[1]
                op2 = get_value(tokens[2], variables)
                result = evaluate_expression(op1, operator, op2)
                variables[left] = result
            else:
                print("Unsupported assignment:", line)
            continue

        else:
            print("Unknown instruction:", line)

    print("\nFinal Variables:", variables)
    if return_value is not None:
        print("Program returned:", return_value)

if __name__ == "__main__":
    main()
