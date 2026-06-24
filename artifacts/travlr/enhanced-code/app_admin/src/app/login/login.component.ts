import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Router } from '@angular/router';
import { TripDataService } from '../services/trip-data.service';
import { User } from '../models/user';

@Component({
  selector: 'app-login',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './login.component.html',
  styleUrl: './login.component.css'
})
export class LoginComponent implements OnInit {
  public formError: string = '';

  credentials = {
    name: '',
    email: '',
    password: ''
  };

  constructor(
    private router: Router,
    private tripDataService: TripDataService
  ) {}

  ngOnInit(): void {}

  public onLoginSubmit(): void {
    this.formError = '';

    if (!this.credentials.email || !this.credentials.password || !this.credentials.name) {
      this.formError = 'All fields are required, please try again';
      return;
    }

    const newUser = {
      name: this.credentials.name,
      email: this.credentials.email
    } as User;

    this.tripDataService.login(newUser, this.credentials.password).subscribe({
      next: (value: any) => {
        console.log('LOGIN RESPONSE:', value);

        if (value && value.token) {
          localStorage.setItem('travlr-token', value.token);
          console.log('TOKEN SAVED:', localStorage.getItem('travlr-token'));
          this.router.navigate(['']);
        } else {
          this.formError = 'Login failed: no token returned';
        }
      },
      error: (error: any) => {
        console.log('LOGIN ERROR:', error);
        this.formError = 'Login failed';
      }
    });
  }
}