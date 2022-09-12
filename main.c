//Evaluation of the arithmetic expression
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void error(char error_message[])
{
    fprintf(stderr, error_message);
    exit(-1);
}

/// Stack Implementation in C using linked list Inspirace: https://www.geeksforgeeks.org/stack-data-structure-introduction-program/
// NENI MOJE TVORBA
struct StackNode
{
    double data;
    struct StackNode *next; // ukazatel na dalsi
};

struct StackNode *create_new_node(double data)
{
    struct StackNode *new_node = (struct StackNode *)malloc(sizeof(struct StackNode)); // alokovani pameti
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

int is_stack_empty(struct StackNode *top)
{
    return !top;
}

void push(struct StackNode **top, double data)
{ // insert
    struct StackNode *new_node = create_new_node(data);
    new_node->next = *top;
    *top = new_node; // top ukazuje na novy prvek
}

double pop(struct StackNode **top)
{ // delete
    if (is_stack_empty(*top))
        exit(-1);
    struct StackNode *temp = *top;
    double popped = temp->data;
    *top = (*top)->next; // top ukazuje na prvek pred smazenem topem
    return popped;
}

double peek(struct StackNode **top)
{ // podivej se do top prvku
    if (is_stack_empty(*top))
        exit(-1);
    double data = (*top)->data;
    return data;
}

///

int get_priority(char operation)
{
    if (operation == '*' || operation == '/')
        return 2;
    if (operation == '-' || operation == '+')
        return 1;
    return 0; // in case of '('
}

double do_the_math(double a, double b, char operation, struct StackNode **stack_operator)
{
    double result = 0;
    switch (operation)
    {
    case '+':
        result = a + b; // associativni
        break;
    case '-':
        result = b - a; // zalezi na poradi tak prvni cislo ktere dostanem bude mensitel a druhe mensenec
        break;
    case '*':
        result = a * b; // associativni
        if (!is_stack_empty((*stack_operator)) && peek(&(*stack_operator)) == '-')
        { // secure, e.g 8-7/1+5 without this would be -4 instead of 6
            pop(&(*stack_operator));
            push(&(*stack_operator), '+');
            result *= -1;
        }
        break;
    case '/':
        if (a == 0)
            error("\nERROR. Division by zero\n");
        result = b / a;
        if (!is_stack_empty((*stack_operator)) && peek(&(*stack_operator)) == '-')
        {
            pop(&(*stack_operator));
            push(&(*stack_operator), '+');
            result *= -1;
        }
        break;
    default: // low chanses to execute
        error("\nERROR. Unknown operator\n");
        break;
    }
    return result;
}

void perfom_operation(struct StackNode **stack_output, struct StackNode **stack_operator) // pointer to stack for perfoming operation between 2 numbers
{
    double num1 = peek(&(*stack_output));
    pop(&(*stack_output));

    double num2 = peek(&(*stack_output));
    pop(&(*stack_output));

    char operation = peek(&(*stack_operator));
    pop(&(*stack_operator));

    double result = do_the_math(num1, num2, operation, &(*stack_operator));
    push(&(*stack_output), result);
}

// JINY POSTUP
// void get_and_change_position_in_file(FILE *file, fpos_t *position, int shift /* posun*/, char operation)
// { // 1 - '+', 0 - '-'
//     if (operation == '+')
//         *position = *position + shift;
//     if (operation == '-')
//         *position = *position - shift;
//     fsetpos(file, position);
// }

void perfom_one_parameter_function(struct StackNode **stack_output, char function[]) // char function[]
{                                                                                    // priority 3
    double number = peek(&(*stack_output));
    pop(&(*stack_output));
    if (!strcmp(function, "cos"))
        number = cos(number);
    else if (!strcmp(function, "sin"))
        number = sin(number);
    else if (!strcmp(function, "tan"))
        number = tan(number);
    else if (!strcmp(function, "log"))
    {
        if (number < 0)
            error("\nERROR. Log call on negative number\n");
        number = log10(number);
    }
    else if (!strcmp(function, "exp"))
        number = exp(number);
    else if (!strcmp(function, "sqrt"))
    {
        if (number < 0)
            error("\nERROR. SQRT call on negative number\n");
        number = sqrt(number);
    }
    else
        error("\nERROR. Unknown function name.\n");
    push(&(*stack_output), number);
}

int main(int argc, char **argv) // "-i" , "vyraz.txt"  
{
    FILE *fp;
    int c;
    char *input_filename;
    fpos_t position; // word position

    struct StackNode *stack_output = NULL;
    struct StackNode *stack_operator = NULL;

    double number;
    double number_fractional_part = 0;

    char math_function[5];
    int counter_multiple_parenthesis = 0;
    int divider_digits_after_decimal_point = 1;
    int counter_for_letters_in_func = 0;
    char avaliable_functions[] = "sin, cos, tan, log, exp, sqrt";

    int flag_if_number = 0;
    int flag_if_not_last_digit = 0;
    int flag_decimal_number = 0;
    int flag_negative_number = 0;
    int flag_first_character_is_operator = 1; // + 8 ...
    int flag_space_after_number = 0;          /// 8 + 7   9
    int flag_if_operator_was_last = 0;        // 12 ++ 7
    int flag_perfom_operation = 0;
    int flag_parenthesis_open = 0;
    int flag_parenthesis_closed = 0;
    int flag_multiple_parenthesis = 0;         // (10 - 9) * (10 - 2)
    int flag_parenthesis_open_if_operator = 0; // (+50)  // for errors
    int flag_function_called = 0;
    int flag_function_name_completed = 0;
    int flag_function_sqrt = 0;
    int flag_parenthesis_for_func = 0;
    int flag_operation_inside_func = 0;

    double result = 0;

    if (!argv[1])
        error("ERROR. File not specified or specified incorrectly\n");
    while (*++argv && **argv == '-')
    { // parameter processing
        if (!strcmp(*argv, "-i"))
        {
            *++argv;
            input_filename = malloc(sizeof(char) * strlen(*argv));
            input_filename = strcpy(input_filename, *argv);
        }
    }

    if (!(fp = fopen(input_filename, "rb")))
        error("ERROR. File not found \n");
    while ((c = getc(fp)) != EOF) //
    {
        printf("%c", c);
        if (isdigit(c))
        {
            flag_first_character_is_operator = 0;
            if (flag_space_after_number && !flag_if_operator_was_last)
                error("\nERROR. Number after number without operator\n");
            flag_space_after_number = 0;
            flag_parenthesis_open_if_operator = 0;
            flag_if_operator_was_last = 0; // rideni flagu
            if (flag_if_not_last_digit)
            {
                if (flag_decimal_number)
                {
                    divider_digits_after_decimal_point *= 10;
                    number_fractional_part = c - '0';
                    number_fractional_part /= divider_digits_after_decimal_point;
                    number += number_fractional_part;
                }
                else
                    number = (number * 10) + c - '0'; // jestli cislo vetse nez jeden znak
            }
            else
            {
                number = c - '0';
                if (flag_negative_number)
                {
                    number *= -1;
                    flag_negative_number = 0;
                }

                flag_if_not_last_digit = 1;
                flag_if_number = 1;
            }
        }
        else if (isalpha(c))
        { // cos sin tan log exp sqrt //ne funguje jestli bude nekolik vnorenych funcki a jestli ve funkce je zbytecne zavorky
            if (!flag_if_operator_was_last && !flag_first_character_is_operator && counter_for_letters_in_func == 0)
                error("\nFunction doesn't have operator before to pefrom\n");
            if (!strchr(avaliable_functions, c))
            {
                fprintf(stderr, "\nERROR. Undefied character %c\n", c);
                exit(-1);
            }
            if (c == 't' && !strcmp(math_function, "sqr"))
            {
                flag_function_sqrt = 1;
                flag_function_name_completed = 0;
            }
                
            math_function[counter_for_letters_in_func] = c;
            // strcat(math_function, c); // nefunguje kvuli tomu ze c je int
            counter_for_letters_in_func++;
            flag_if_operator_was_last = 0;

            // JINY POSTUP
            // fgetpos(fp, &position);
            // get_and_change_position_in_file(fp, &position, 1, '-');
            // fgets(math_function, 4, fp);
            // get_and_change_position_in_file(fp, &position, 3, '+');

            if (flag_function_name_completed)
                error("\nFunction called without resulting previous\n");
            if (counter_for_letters_in_func == 4 && flag_function_sqrt)
                flag_function_name_completed = 1;
            else if (counter_for_letters_in_func == 3 && !flag_function_sqrt)
                flag_function_name_completed = 1; 
            flag_function_called = 1;
            flag_if_not_last_digit = 0;
            flag_if_number = 0;
        }
        else
        {

            switch (c)
            {
            case '.':
                if (flag_if_number && flag_if_not_last_digit)
                    flag_decimal_number = 1;
                else
                    error("\nERORR. Operator . not in right place\n");
                break;
            case ' ':
                flag_space_after_number = 1;
                flag_if_not_last_digit = 0;
                break;
            case '\n':
                flag_if_not_last_digit = 0;
                break;
            case '+':
                if (flag_first_character_is_operator)
                    error("\nERROR. Operator before any number number\n");
                if (flag_if_operator_was_last)
                    error("\nERROR. Operator after operator without number\n");
                flag_if_operator_was_last = 1;
                if (flag_parenthesis_open_if_operator)
                    error("\nERROR. Operator after parenthesis without number before\n");
                if (flag_parenthesis_for_func)
                    flag_operation_inside_func = 1;
                if (flag_if_number)
                { // situation when after number or operator no spaces
                    push(&stack_output, number);
                    flag_if_not_last_digit = 0;
                    flag_if_number = 0;
                }
                if (!is_stack_empty(stack_operator) && get_priority(peek(&stack_operator)) >= 1)
                    perfom_operation(&stack_output, &stack_operator);
                push(&stack_operator, '+');
                break;
            case '-':
                if (flag_if_operator_was_last || flag_first_character_is_operator)
                    flag_negative_number = 1;
                if (flag_parenthesis_open_if_operator)
                    error("\nERROR. Operator after parenthesis without number before\n");
                if (flag_parenthesis_for_func)
                    flag_operation_inside_func = 1;
                flag_if_operator_was_last = 1;
                if (flag_if_number)
                { // situation when after number or operator no spaces
                    push(&stack_output, number);
                    flag_if_not_last_digit = 0;
                    flag_if_number = 0;
                }
                if (!is_stack_empty(stack_operator) && get_priority(peek(&stack_operator)) >= 1 && !flag_negative_number)
                    perfom_operation(&stack_output, &stack_operator);
                if (!flag_negative_number)
                    push(&stack_operator, '-');
                break;
            case '*':
                if (flag_first_character_is_operator)
                    error("\nERROR. Operator before any number number\n");
                if (flag_if_operator_was_last)
                    error("\nERROR. Operator after operator without number\n");
                if (flag_parenthesis_open_if_operator)
                    error("\nERROR. Operator after parenthesis without number before\n");
                if (flag_parenthesis_for_func)
                    flag_operation_inside_func = 1;
                flag_if_operator_was_last = 1;
                if (flag_if_number)
                { // situation when after number or operator no spaces
                    push(&stack_output, number);
                    flag_if_not_last_digit = 0;
                    flag_if_number = 0;
                }
                if (!is_stack_empty(stack_operator) && get_priority(peek(&stack_operator)) >= 2)
                    perfom_operation(&stack_output, &stack_operator);
                push(&stack_operator, '*');
                break;
            case '/':
                if (flag_first_character_is_operator)
                    error("\nERROR. Operator before any number number\n");
                if (flag_if_operator_was_last)
                    error("\nERROR. Operator after operator without number\n");
                if (flag_parenthesis_open_if_operator)
                    error("\nERROR. Operator after parenthesis without number before\n");
                if (flag_parenthesis_for_func)
                    flag_operation_inside_func = 1;
                flag_if_operator_was_last = 1;
                if (flag_if_number)
                { // situation when after number or operator no spaces
                    push(&stack_output, number);
                    flag_if_not_last_digit = 0;
                    flag_if_number = 0;
                }
                if (!is_stack_empty(stack_operator) && get_priority(peek(&stack_operator)) >= 2)
                    perfom_operation(&stack_output, &stack_operator);
                push(&stack_operator, '/');
                break;
            case '(':
                flag_if_operator_was_last = 1;
                flag_if_not_last_digit = 0;
                // if(flag_parenthesis_for_func) error("\nOpened patenthesis in func weren't closed\n");
                if (!flag_function_called)
                {
                    flag_parenthesis_open_if_operator = 1;
                    push(&stack_operator, '(');
                    if (flag_parenthesis_open == 1)
                    { // previous ( made parehtesis opened so it's multiple
                        flag_multiple_parenthesis = 1;
                        counter_multiple_parenthesis += 1;
                    }
                    flag_parenthesis_open = 1;
                }
                else
                {
                    if (flag_parenthesis_for_func == 1)
                    { // previous ( made parehtesis opened so it's multiple
                        flag_multiple_parenthesis = 1;
                        counter_multiple_parenthesis += 1;
                    }
                    push(&stack_operator, '(');
                    flag_parenthesis_for_func = 1;
                }

                break;
            case ')':
                if (!flag_function_called)
                {
                    if (!flag_parenthesis_open)
                        error("\nERROR. Closing brackets without opening \n");
                    flag_parenthesis_closed = 1;
                    flag_if_not_last_digit = 0;
                }
                else
                {
                    if (flag_if_number)
                        push(&stack_output, number);
                    if (flag_parenthesis_for_func && flag_operation_inside_func)
                    {
                        flag_if_number = 0;
                        flag_if_not_last_digit = 0;
                        while (peek(&stack_operator) != '(')
                            perfom_operation(&stack_output, &stack_operator);
                        if (!flag_multiple_parenthesis)
                        {
                            flag_operation_inside_func = 0;
                            flag_parenthesis_for_func = 0;
                        }
                        else if (counter_multiple_parenthesis == 0)
                        {
                            flag_multiple_parenthesis = 0;
                            flag_parenthesis_for_func = 0;
                        }
                        else
                            counter_multiple_parenthesis -= 1;
                        // if (!is_stack_empty(stack_operator) && peek(&stack_operator) == '(' && counter_multiple_parenthesis == 0 && flag_multiple_parenthesis)
                        //     error("\nOpened bracket inside function weren't closed\n");
                    }
                    if (peek(&stack_operator) == '(')
                        pop(&stack_operator); // double checking before deleting
                    if (!flag_multiple_parenthesis)
                    {
                        perfom_one_parameter_function(&stack_output, math_function);
                        counter_for_letters_in_func = 0;
                        memset(math_function, 0, sizeof math_function);
                        flag_function_name_completed = 0;
                        flag_function_called = 0;
                        flag_if_not_last_digit = 0;
                        flag_if_number = 0;
                        flag_decimal_number = 0;
                        divider_digits_after_decimal_point = 1;
                        flag_parenthesis_for_func = 0;
                    }

                    // flag_parenthesis_for_func = 0;
                }
                break;
            default:
                fprintf(stderr, "\nERROR. UNDEFIED OPERATOR %c\n", c);
                exit(-1);
                break;
            }
        }
        if (!flag_if_not_last_digit && flag_if_number)
        {
            push(&stack_output, number);
            flag_if_number = 0;
            flag_decimal_number = 0;
            divider_digits_after_decimal_point = 1; // reset
        }

        if (flag_parenthesis_open && flag_parenthesis_closed)
        {
            while (peek(&stack_operator) != '(')
                perfom_operation(&stack_output, &stack_operator);
            if (peek(&stack_operator) == '(')
                pop(&stack_operator); // double checking before deleting
            else
                exit(-1);
            flag_parenthesis_closed = 0;
            if (!flag_multiple_parenthesis)
                flag_parenthesis_open = 0;
            else if (counter_multiple_parenthesis == 0)
            {
                flag_multiple_parenthesis = 0;
                flag_parenthesis_open = 0;
            }
            else
                counter_multiple_parenthesis -= 1;
        }
    }
    if (flag_if_not_last_digit == 1)
    { // if file ends without ' ' we have last digit
        flag_decimal_number = 0;
        divider_digits_after_decimal_point = 1;
        push(&stack_output, number);
    }

    if (flag_if_operator_was_last) // checking for last operator e.g 8 + 9 +
        error("\nERROR. Last operator without number\n");
    if (flag_parenthesis_open && !flag_parenthesis_closed)
        error("\nERROR. Opened brackets weren't closed\n");
    if (flag_decimal_number)
        error("\nERORR. Number with '.' doesn't have fractional part\n");
    // push(&stack_output, pop(&stack_operator));
    while (!is_stack_empty(stack_operator))
        perfom_operation(&stack_output, &stack_operator);
    printf("\n");
    result = pop(&stack_output);
    if (!is_stack_empty(stack_output) || !is_stack_empty(stack_operator))
        error("\nERROR.Something went wrong. Check your expression\n");
    printf("Final result is %.3f\n", result);
    printf("Output stack is Empty? %d\n", is_stack_empty(stack_output));
    printf("Operator stack is Empty? %d\n", is_stack_empty(stack_operator));
    fclose(fp);
    // free(input_filename);
    return 0;
}