int main() {
    int x = 0;
    int y = 0;

    for (int i = 0; i < 200; i++) {
        if (x == y) {
            x++;
        }
        else {
            y++;
        }
        int z = x+7;
    }
    return z;
}